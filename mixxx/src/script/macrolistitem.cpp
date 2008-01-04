#include "macrolistitem.h"

MacroListItem::MacroListItem(MacroList* parent, Macro* macro) :
	Q3ListViewItem(parent, "Dummy") {

	m_macro = macro;
		
	setText(0, macro->getName());
	setRenameEnabled(0, true);
}

MacroListItem::~MacroListItem() {
}

void MacroListItem::okRename(int col) {
	Q3ListViewItem::okRename(col);
	m_macro->setName(text(0));
}

Macro* MacroListItem::getMacro() {
	return m_macro;
}
