function class(base, init)
   local c = {}    -- a new class instance
   local mt = {}
   if not init and type(base) == 'function' then
      init = base
      base = nil
   elseif type(base) == 'table' then
      mt.__index = base
   end
   c.__index = c

   mt.__call = function(class_tbl, ...)

   local obj = {}
   setmetatable(obj,class_tbl)
   if class_tbl.init then
      class_tbl.init(obj,...)
   else
      if base and base.init then
      base.init(obj, ...)
      end
   end
   return obj
   end
   c.init = init
   --[[c.is_a = function(self, klass)
      local m = getmetatable(self)
      while m do
         if m == klass then return true end
         m = m._base
      end
      return false
   end]]
   setmetatable(c, mt)
   return c
end
