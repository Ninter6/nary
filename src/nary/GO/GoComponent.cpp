//
//  GOComponent.cpp
//  pxpls
//
//  Created by Ninter6 on 2023/11/4.
//

#include "GoComponent.hpp"

namespace nary {

ComponentWapper::ComponentWapper(const ComponentWapper& o)
: m_Compo(o.copy()), copy_fn(o.copy_fn) {}

ComponentWapper& ComponentWapper::operator=(const ComponentWapper& o) {
    m_Compo.reset(o.copy());
    copy_fn = o.copy_fn;
    return *this;
}

}
