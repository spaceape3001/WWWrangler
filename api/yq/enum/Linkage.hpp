////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "yq/util/Enum.hpp"
#include "yq/meta/MetaValue.hpp"


YQ_ENUM(Linkage, ,
    None,
    Any,
    Indirect,
    Direct
)

MV_DECLARE(Linkage)
