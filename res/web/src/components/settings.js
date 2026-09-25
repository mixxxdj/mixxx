import Sortable from "sortablejs";

/**
 * Settings component — deck visibility/ordering via Sortable.
 *
 * Methods are designed to be mixed into the main Alpine component (they use `this`).
 */
export default {
    init_settings_decks_sortable() {
        Sortable.create(this.$refs.decksOrder, {
            animation: 150,
            handle: "input",
            onEnd: (evt) => {
                const movedItem = this.settings.decks.splice(evt.oldIndex, 1)[0];
                this.settings.decks.splice(evt.newIndex, 0, movedItem);
            },
        });
    },
};
