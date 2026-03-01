#include "rendergraph/attributeset.h"

#include "backend/baseattributeset.h"

using namespace rendergraph;

AttributeSet::AttributeSet(std::initializer_list<AttributeInit> list, const std::vector<QString>&)
        : BaseAttributeSet(list) {
    // names are not used in scenegraph, but needed in the API for the opengl backend
}
