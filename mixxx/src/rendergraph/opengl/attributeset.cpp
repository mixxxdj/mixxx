#include "rendergraph/attributeset.h"

using namespace rendergraph;

AttributeSet::AttributeSet(std::initializer_list<AttributeInit> list,
        const std::vector<QString>& names)
        : BaseAttributeSet(list, names) {
}
