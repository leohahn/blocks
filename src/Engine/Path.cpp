#include "Han/Path.hpp"
#include "Han/Utils.hpp"

StringView 
Path::GetExtension(const char* path)
{
    StringView view;
    StringView path_view(path);

    size_t dot_index;
    if (!StringUtils::FindFromRight(path_view, '.', &dot_index)) {
        return view;
    }

    view = StringView(&path[dot_index], path_view.len - dot_index);
    return view;
}