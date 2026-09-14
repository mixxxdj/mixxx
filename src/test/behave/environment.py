import functools
import os
import re
import shutil
import time
from behave.model import ScenarioOutline
from behave.model_core import Status


def _parse_gh_issue(tags):
    for tag in tags:
        m = re.match(r"xfail\(gh_issue=(\d+)\)", tag)
        if m:
            return int(m.group(1))
    return None


def _is_expected_failure_or_flaky(tags):
    return any(t.startswith("xfail") or t.startswith("xpass") for t in tags)


def _reset_session(context):
    """Stop the running Mixxx instance and clear the whole session grouping so
    the next scenario (or retry attempt) starts from a clean process and a
    fresh profile. A scenario that fails may leave Mixxx in an unknown state,
    so it must never be reused as a grouped instance."""
    session = context._session
    # Stop both the session instance and whatever context.mixxx points at:
    # after a failed start() the two diverge (context holds the never-ready
    # process), and a live stray keeps port 9000 occupied.
    for mixxx in (session.get("mixxx"), getattr(context, "mixxx", None)):
        if mixxx is not None:
            try:
                mixxx.stop()
            except Exception:
                pass
    session["mixxx"] = None
    session["rpc"] = None
    session["ready_key"] = None
    session["registered_devices"] = None
    session["active_profile_type"] = None
    session["profile_dir"] = None
    context.mixxx = None
    context.mixxx_rpc = None


def patch_scenario_with_autoretry(context, scenario, max_attempts=3):
    """Monkey-patches :func:`~behave.model.Scenario.run()` to auto-retry a
    scenario that fails. Based on behave.contrib.scenario_autoretry but also:
    - Collects retry results and appends a single entry to context.results
    - Sets had_failure only when all retries are exhausted
    """
    def scenario_run_with_retries(scenario_run, *args, **kwargs):
        attempts = []
        for attempt in range(1, max_attempts+1):
            failed = scenario_run(*args, **kwargs)
            if any(t.startswith("xfail") for t in scenario.tags):
                return False    # -- NOT-FAILED = EXPECTED FAILURE
            if not failed:
                if attempt > 1:
                    message = u"AUTO-RETRY SCENARIO PASSED (after {0} attempts)"
                    print(message.format(attempt))
                return False    # -- NOT-FAILED = PASSED
            # -- SCENARIO FAILED:
            if attempt < max_attempts:
                print(u"AUTO-RETRY SCENARIO (attempt {0})".format(attempt))
                _reset_session(context)
        # -- All attempts exhausted (or final attempt failed): poison the
        #    session so a later scenario never inherits partial state.
        if _is_expected_failure_or_flaky(scenario.tags):
            return False    # -- NOT-FAILED = EXPECTED FAILURE
        message = u"AUTO-RETRY SCENARIO FAILED (after {0} attempts)"
        print(message.format(max_attempts))
        _reset_session(context)
        context._session["had_failure"] = True
        return True

    if isinstance(scenario, ScenarioOutline):
        scenario_outline = scenario
        for scenario in scenario_outline.scenarios:
            scenario_run = scenario.run
            scenario.run = functools.partial(scenario_run_with_retries, scenario_run)
    else:
        scenario_run = scenario.run
        scenario.run = functools.partial(scenario_run_with_retries, scenario_run)


def before_all(context):
    context.chapters = []
    context.results = []
    context._session = {
        "profile_dirs": [],
        "active_profile_type": None,
        "profile_dir": None,
        "mixxx": None,
        "rpc": None,
        "ready_key": None,
        "registered_devices": None,
        "had_failure": None,
    }


def before_feature(context, feature):
    max_attempts = context.config.userdata["retry_max_attempts"]
    for scenario in feature.walk_scenarios():
        patch_scenario_with_autoretry(context, scenario, max_attempts)


def before_scenario(context, scenario):
    session = context._session
    if session.get("mixxx") is not None:
        context.mixxx = session["mixxx"]
        context.mixxx_rpc = session["rpc"]
        context.profile_dir = session["profile_dir"]
        context.active_profile_type = session["active_profile_type"]

    context._soundMockDevices = None
    context._remembered = {}

    scenario.start_at = time.time()
    if session["had_failure"] and context.config.userdata["fail_early"]:
        scenario.skip(reason="Skipped due to previous failure")


def after_scenario(context, scenario):
    outcome = str(scenario.status).split('.')[1].lower()

    failed = scenario.status in [Status.failed, Status.error]
    is_xfail = any(map(lambda t: t.startswith("xfail"), scenario.tags))
    if failed and is_xfail:
        outcome = "expected failure"
    elif is_xfail:
        outcome = "unexpected pass"
    elif failed and "xpass" in scenario.tags:
        outcome = "flaky"

    if failed and _is_expected_failure_or_flaky(scenario.tags):
        # Expected failures and flaky scenarios are not genuine failures:
        # mark them as passed so behave's summary does not list them as failed.
        scenario.set_status(Status.passed)

    time.sleep(1) # Allow the final state to be visible on screen

    # Reset transient state that a scenario may have left behind (e.g. a
    # maximized library) so the next scenario reusing this Mixxx instance
    # starts from the defaults rather than inheriting it.
    rpc = context._session.get("rpc")
    if rpc is not None:
        try:
            rpc.command("setControlValue", "[Skin],show_maximized_library,0")
        except Exception:
            pass

    scenario.end_at = time.time()
    scenario.outcome = outcome

    # Collect the actual exception line from each failing step's error
    # message. Behave stores the full traceback in step.error_message, so the
    # meaningful line (e.g. "AssertionError: ...", "RuntimeError: ...") is the
    # last non-empty line, not the "Traceback (most recent call last):" header.
    # Successes and expected failures are omitted.
    errors = []
    for step in scenario.steps:
        if step.status not in (Status.failed, Status.error):
            continue
        msg = (getattr(step, "error_message", "") or "").strip()
        if not msg:
            continue
        error_line = next(
            (line.strip() for line in reversed(msg.splitlines()) if line.strip()),
            None,
        )
        if error_line:
            errors.append(error_line)

    context.results.append({
        "title": f"{scenario.feature.name} > {scenario.name} [{getattr(scenario, 'outcome', 'skipped')}]",
        "feature": scenario.feature.name,
        "name": scenario.name,
        "outcome": outcome,
        "tags": list(scenario.tags),
        "gh_issue": _parse_gh_issue(scenario.tags),
        "start_time": scenario.start_at,
        "end_time": time.time(),
        "error": " / ".join(errors),
    })


def after_all(context):
    session = context._session
    mixxx = session.get("mixxx")
    if mixxx is not None:
        try:
            rpc = session.get("rpc")
            if rpc is not None:
                rpc.quit()
                time.sleep(1)
        except Exception:
            pass
        mixxx.stop()

    # A failed start() leaves context.mixxx pointing at a never-ready
    # process the session never learned about; stop it too, or it outlives
    # the run while holding port 9000.
    stray = getattr(context, "mixxx", None)
    if stray is not None and stray is not mixxx:
        try:
            stray.stop()
        except Exception:
            pass

    for d in session.get("profile_dirs", []):
        shutil.rmtree(d, ignore_errors=True)
