#ifndef SCRIPT_MACROLISTITEM_H
#define SCRIPT_MACROLISTITEM_H

#include <q3listview.h>

#include "macrolist.h"
#include "macro.h"

class MacroListItem : public Q3ListViewItem {
	public:
		MacroListItem(MacroList* parent, Macro* macro);
		~MacroListItem();

		Macro* getMacro();
	protected:
		virtual void okRename(int col);
	private:
		Macro* m_macro;
};

#endif
