#!/usr/bin/env python3
import sys
import os
import signal
import re

try:
    # import cpuset
    from cpuset.commands import set as ccset
    from cpuset import cset
    from cpuset.commands import proc
    from cpuset.util import CpusetExists

except ImportError as e:
    sys.stderr.write(e.msg)
    sys.stderr.write("\nplease install cpuset\n")
    sys.exit(1)


import argparse
import logging
import subprocess
from shutil import which

logging.basicConfig(level=logging.INFO)
log = logging.getLogger("mixxx-isolate")


def selectCpus(userCpu=-1):
    """
    Selects the most fitting cpu and it's ht companions.
    Returns {"use": cpuid, "isolate": [cpuid, ...], "system": [cpuid, ...]}
    """
    cpuinfo = ""
    with open("/proc/cpuinfo") as fp:
        cpuinfo = fp.read(-1)

    cpuToCore = {}
    currentCpu = -1
    lastCore = -1
    for line in cpuinfo.splitlines():
        if not line.strip():
            continue
        topic, data = line.split(":")
        topic = topic.strip()
        if topic == "processor":
            currentCpu = int(data)
        elif topic == "core id":
            coreId = int(data)
            cpuToCore[currentCpu] = coreId
            lastCore = max(lastCore, coreId)

    useCore = -1
    if userCpu == -1:
        useCore = lastCore
    else:
        useCore = cpuToCore.get(userCpu, -1)
        if useCore == -1:
            print("Selected CPU does not exist. Using autodetect.")
            useCore = lastCore

    useCpu = -1
    isolate = []
    system = []
    for cpu, core in cpuToCore.items():
        if core == useCore and useCpu == -1:
            useCpu = cpu
        elif core == useCore:
            isolate.append(cpu)
        else:
            system.append(cpu)

    return {"use": useCpu, "isolate": isolate, "system": system}


def cpusetname(name):
    if name and name[0] == "/":
        return name
    return "/%s" % name


def deleteCpuset(name, debug):
    try:
        setInstance = cset.unique_set(name)
    except cset.CpusetNotFound:
        return True
    # move processes until empty
    processes = 1
    while processes:
        try:
            log.debug("Move processes from cpuset %s %s", setInstance, name)
            proc.move(setInstance, "/", None, False)
            processes = setInstance.tasks
            log.debug("Processes left: %s", processes)
            break
        except BrokenPipeError:
            continue
        except Exception as e:
            log.exception(e)
            print(e)

    ccset.destroy(name)
    return True


def teardown(args, isolate):
    cset.CpuSet(None)

    if not args.no_offline:
        if args.cleanup:
            # since our detection algorithm can't detect isolation cpus
            # form offline CPUs, we will just bring up all cores on teardown
            for fname in os.listdir("/sys/devices/system/cpu"):
                try:
                    if re.match("cpu[0-9]+", fname):
                        with open(
                            "/sys/devices/system/cpu/%s/online" % fname, "w"
                        ) as fp:
                            fp.write("1")
                except OSError:
                    pass
                except Exception as e:
                    print(e)

        for iso in isolate:
            with open("/sys/devices/system/cpu/cpu%d/online" % iso, "w") as fp:
                fp.write("1")

    log.info("deactivate cpusets")
    deleteCpuset(args.cpuset_engine, args.debug)
    deleteCpuset(args.cpuset_isolation, args.debug)
    deleteCpuset(args.cpuset_system, args.debug)

    log.info("Cleanup finished")
    print("STATUS: CLEAN")


def createcpuset(name, cpuspec, memspec, cx, mx):
    try:
        ccset.create(name, cpuspec, memspec, cx, mx)
    except CpusetExists:
        pass
    except Exception as e:
        raise e


def joinCpuIds(lst):
    return ",".join((str(x) for x in lst))


def isolateSystem(args):
    conf = selectCpus(args.cpu)

    memspec = "0"
    log.info("created cpuset groups")
    try:
        createcpuset(
            args.cpuset_engine, str(conf["use"]), memspec, False, False
        )
        createcpuset(
            args.cpuset_system,
            joinCpuIds(conf["system"]),
            memspec,
            False,
            False,
        )
        if args.isolate and conf["isolate"]:
            createcpuset(
                args.cpuset_isolation,
                joinCpuIds(conf["isolate"]),
                memspec,
                False,
                False,
            )
    except Exception as instance:
        teardown(args, conf["isolate"])
        # unroll
        log.exception(instance)
        log.critical("Failed to create shield, hint: do other cpusets exist?")
        sys.exit(23)

    # move all processes to the system cpuset
    log.info("move processes into system group")
    root_tasks = cset.unique_set("/").tasks
    while True:
        try:
            proc.move("root", args.cpuset_system, root_tasks, args.debug)
            break
        except cset.CpusetException:
            pass

    if not args.no_offline:
        for iso in conf["isolate"]:
            with open("/sys/devices/system/cpu/cpu%d/online" % iso, "w") as fp:
                fp.write("0")

    # make the task file own by the user, so the engine can add itself later
    taskFile = (
        cset.CpuSet.basepath + args.cpuset_engine + cset.CpuSet.tasks_path
    )
    os.chown(taskFile, args.uid, -1)

    print("STATUS: ISOLATION")

    def signalHandler(signum, frame):
        teardown(args, conf["isolate"])

    signal.signal(signal.SIGUSR1, signalHandler)

    while True:
        try:
            s = input(">>>")
            if s == "exit":
                break
            else:
                print("unknown command")
        except KeyboardInterrupt:
            break
        except EOFError:
            break

    teardown(args, conf["isolate"])
    sys.exit(0)


def selectCommand(args):
    if sys.stdin.isatty() and which("sudo"):
        return ["sudo", "-u", "root", "--preserve-env=PYTHONPATH,PATH"]

    if which("pkexec"):
        return ["pkexec", "--user", "root"]

    if which("qsudo"):
        return ["qsudo", "-u", "root", "--preserve-env=PYTHONPATH,PATH"]

    log.error("No supported sudo/pkexec/gksudo/ksudo executable found")
    return None


def elevatePermissions(args):
    helperArgs = [sys.executable, os.path.abspath(__file__), "--worker"]

    if args.cleanup:
        helperArgs.append("--cleanup")
    else:
        helperArgs += [
            "--cpuset_engine",
            args.cpuset_engine,
            "--cpuset_system",
            args.cpuset_system,
            "--cpuset_isolation",
            args.cpuset_isolation,
            "--cpu-id",
            str(args.cpu),
            "--uid",
            str(os.getuid()),
        ]
        if not args.isolate:
            helperArgs.append("--no-isolate-ht")

    if args.debug:
        helperArgs.append("--debug")

    command = selectCommand(args)
    if not command:
        return None

    fullCommand = command + helperArgs
    log.info("Execute %s", fullCommand)

    return subprocess.Popen(
        fullCommand,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=sys.stderr,
    )


def startHelper(args):
    """Execute $self with adjusted arguments under privilege elevation"""

    helper = elevatePermissions(args)
    mixxxProc = None
    if not helper:
        log.critical("Could not spawn with elevated permissions")
    while True:
        try:
            helper.poll()
            if helper.returncode == 127:
                # user aborted password request, nothing happend so its
                # save to exit
                log.error("Could not run elevate process. Aborting")
                sys.exit(1)
            elif helper.returncode == 23:
                # Error setting up isolation
                log.critical("Error creating isolation. Aborting")
                sys.exit(1)
            elif helper.returncode is not None:
                # helper process died
                log.info(helper.stdout.read(-1))
                log.error(
                    "Helper process died: %s."
                    "Cleanup might be required required",
                    helper.returncode,
                )
                break

            line = helper.stdout.readline().decode("utf-8", "ignore")
            if line.startswith("STATUS:"):
                status = line[7:].strip()
                print("GOT STATUS:" + status)
                if status == "ISOLATION":
                    log.info("Isolation setup complete. Starting mixxx....")
                    mixxxProc = subprocess.Popen(
                        [args.mixxx, "--engineCpuSet", args.cpuset_engine]
                        + args.mixxx_args
                    )
                    mixxxProc.wait()
                    log.info("Mixxx finished")
                    helper.stdin.write("exit\n".encode("ASCII"))
                    try:
                        # this might likely not work
                        helper.send_signal(signal.SIGUSR1)
                    except PermissionError:
                        pass
                    break
                elif status == "CLEAN":
                    # cleanup finished.
                    log.info("Exit successfully")
                    sys.exit(0)
            else:
                print(line)

        except KeyboardInterrupt:
            log.info("Received keyboard interrupt. Stopping")
            if mixxxProc and mixxxProc.returncode is None:
                mixxxProc.send_signal(signal.SIGINT)
            break
        except Exception as e:
            log.exception(e)
    # trigger system cleanup
    try:
        # this might likely not work
        helper.send_signal(signal.SIGUSR1)
    except PermissionError:
        pass

    helper.stdin.write("exit\n".encode("ASCII"))
    helper.communicate()
    log.info("Exit successfully")
    sys.exit(0)


def main():
    defaultMixxx = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "mixxx"
    )

    parser = argparse.ArgumentParser(
        description="""Runs mixxx with isolated engine on a dedicated CPU.
    You can pass mixxx arguments using ' -- 'after mixxx-isolate arguments.

    This program will seperate one CPU core from the system and place the
    mixxx engine thread on a dedicated CPU.
    Hyperthreading siblings will be isolated and taken offline if possible.

    The normal system state is restored when mixxx is closed.
    """
    )
    parser.add_argument(
        "--cpuset_engine",
        dest="cpuset_engine",
        default="mixxx",
        help="cpuset name for the mixxx engine",
        type=cpusetname,
    )
    parser.add_argument(
        "--cpuset_system",
        dest="cpuset_system",
        default="system",
        help="cpuset name for the mixxx engine",
        type=cpusetname,
    )
    parser.add_argument(
        "--cpuset_isolation",
        dest="cpuset_isolation",
        default="mixxx-isolate",
        help="cpuset name for the mixxx engine",
        type=cpusetname,
    )
    parser.add_argument(
        "--no-isolate-ht",
        dest="isolate",
        action="store_false",
        default=True,
        help="don't isolate hyperthreading partner (not recommended)",
    )
    parser.add_argument(
        "--no-offline",
        dest="no_offline",
        action="store_true",
        help="don't take hyperthreading cpus offline",
    )
    parser.add_argument(
        "--cpu-id",
        dest="cpu",
        type=int,
        default=-1,
        help="ID of the cpu to use for the engine. -1 for autoselect",
    )
    parser.add_argument(
        "--cleanup",
        dest="cleanup",
        action="store_true",
        help="Cleanup old isolation settings",
    )
    parser.add_argument(
        "--mixxx", dest="mixxx", default=defaultMixxx, help="Mixxx executable"
    )
    parser.add_argument(
        "mixxx_args", nargs="*", help="Arguments passed to mixxx"
    )
    parser.add_argument(
        "--debug",
        dest="debug",
        action="store_true",
        default=False,
        help="Debug output",
    )
    parser.add_argument(
        "--worker",
        dest="worker",
        action="store_true",
        default=False,
        help=argparse.SUPPRESS,
    )
    parser.add_argument(
        "--uid", dest="uid", default=None, type=int, help=argparse.SUPPRESS
    )

    args = parser.parse_args()

    # initialize CpuSet
    cset.CpuSet(None)

    if args.debug:
        logging.root.setLevel(logging.DEBUG)

    if args.worker:
        if args.cleanup:
            log.info("Cleanup old isolation")
            teardown(args, [])
            log.info("done")
            sys.exit(0)
        if not args.uid:
            log.critical("No uid passed to worker.")
            sys.exit(1)
        isolateSystem(args)
    else:
        startHelper(args)


if __name__ == "__main__":
    main()
