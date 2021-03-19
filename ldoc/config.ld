project = 'Ovis Engine'
title = 'Ovis Engine Lua API'
format = 'discount'
topics = 'doc_topics'
use_markdown_titles = true
kind_names = {topic='Manual'}
template = true
style = true
merge = true

custom_tags = {
  { 'cppsetup', hidden = true }
}

custom_display_name_handler = function(item, default_handler)
  if item.module then
    local name = item.name:gsub(item.module.name .. '[:\.]', '')
    if item.type == 'function' then
      name = name .. ' ' .. item.args
      -- if item.retgroups then
      --   for i, group in ldoc.ipairs(groups)
      --   end
      --   name = name .. 'plus return'
      -- end
    end
    return name
  end
  return default_handler(item)
end