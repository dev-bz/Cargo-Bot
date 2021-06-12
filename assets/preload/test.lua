DeltaTime=0.05
ElapsedTime=0.0
WIDTH=768
HEIGHT=1024
POLYGON=3
CIRCLE=2
KINEMATIC=1
STATIC=0
DYNAMIC=2
BEGAN=1
MOVING=2
ENDED=3
CENTER=0
CORNER=1
CORNERS=2
FULLSCREEN=1
PORTRAIT_ANY=1

ENCODE=0
DATA=1
SOUND_BLIT=2
SOUND_EXPLODE=3
SOUND_HIT=4
SOUND_JUMP=5
SOUND_PICKUP=6
SOUND_POWERUP=7
SOUND_RANDOM=8
SOUND_SHOOT=9
touch={state=BEGAN,x=200,y=450}
parameter={}
function parameter:watch(...)print(...)end
--Java Start
function isRecording()return nil end
function startRecording()end
function stopRecording()end
function background()end
function displayMode(mode)
	print("displayMode-"..mode)
end
function spriteMode(mode)sMode=mode end
function sprite(img,x,y,w,h)
	--print("sprite-"..img)
		local tex=Texture[img]
		if not tex then tex={} end
		local tid=tex.t
		local ww
		local hh
		if not tid then
			ww,hh,tid=loadTexture(img)
			Texture[img]={t=tid,w=ww,h=hh}
		end
		if sMode == CORNER then
			x=x+w*0.5
			y=y+h*0.5
    elseif sMode == CORNERS then
			w=w-x
			h=h-y
      x=x+w*0.5
			y=y+h*0.5
		end
		gldraw(tid,x,y,w,h)
end
function color(r,g,b,a)
	--print(r,g,b,a)
	if not a then
		error("Not Alpha")
	end
	return {r=r,g=g,b=b,a=a}
end
function noTint()tint(255,255,255,255)end
function soundBufferSize()
	return 200,200
end
fsize=50
if nil==sound then
function sound(data,code,value)
SAVE_DATA=data or 'no data'
SAVE_CODE=code or 'no code'
SAVE_SIZE=value or 'no value'
end
end
function font(fstr)
currentFont=fstr
end
function fontSize(size)
fsize=size
end
function noFill()fill(255,255,255,255)end
tMode=0
function textMode(mo)tMode=mo end
wrapwidth=-1
function textWrapWidth(wWidth)
wrapwidth=wWidth
end
function textAlign()end
charScales={}
for i=1,127,1 do
charScales[i]=charSize(i)
end

function textSize(str)
	local lc=1
	local ls=0
	local mx=0
	local st=0
	local ds=1
	local o=0.0
	local x=0.0
	if type(str)=="number" then
		local tmp=tostring(str)
		for i=1,#tmp,1 do
			local v=tmp:byte(i)
			mx=mx+charScales[v]*fsize
		end
		lc=1
	else
		for i=1,#str,1 do
			local v=str:byte(i)
			if v > 223 then v=127 elseif v>127 then v=1 end
			x=x+charScales[v]*fsize
			if v<=32 then
			  st=i
			  o=x
			end
			if v==10 then
				ds=st+1
				lc=lc+1
				if mx<o then mx=o end
				ls=0
				x=0.0
				o=0.0
			elseif wrapwidth>0 and x > wrapwidth then
				if st<ds then
					st=i-1
					o=x-charScales[v]*fsize
				end
				if mx<o then mx=o end
				x=x-o
				o=0.0
				ls=i-st
				ds=st+1
				lc=lc+1
			else
				ls=ls+1
			end
		end
	end
	if mx<x then mx=x end
	return math.floor(mx+0.5),lc*fsize
end
function text(str,x,y)
	tw,th=textSize(str)
	local lx=x
	local ly=y+th
	local bx=lx
	local ds=1
	local st=0
	local ls=0
	local oo=0.0
	local xx=0.0

	if tMode==CENTER then
		lx=x-tw*0.5
		ly=y+th*0.5
		bx=lx
	end
	if type(str)=="number" then
	--glRect(0,lx,ly,fsize*0.45,fsize)
		local num=tostring(str)
		for i=1,#num,1 do
			local tc=num:byte(i)
			local ws=charScales[tc]*fsize
			glRectText(tc,0,lx,ly,ws,fsize)
			lx=lx+ws
		end
	else
		for i=1,#str,1 do
			local v=str:byte(i)
			if v > 223 then v=127 elseif v>127 then v=1 end
			xx=xx+charScales[v]*fsize
			if v<=32 then
			  st=i
			  oo=xx
			end
			if v==10 then
				for j=ds,st,1 do
					local tc=str:byte(j)
					if tc > 223 then tc=127 elseif tc>127 then tc=1 end
					local ws=charScales[tc]*fsize
					if tc>32 then
						glRectText(tc,0,lx,ly,ws,fsize)
					end
					lx=lx+ws
				end
				ls=0
				xx=0.0
				oo=0.0
				ds=st+1
				lx=bx
				ly=ly-fsize
			elseif wrapwidth>0 and xx > wrapwidth then
				if st<ds then
					st=i-1
					oo=xx-charScales[v]*fsize
				end
				for j=ds,st,1 do
					local tc=str:byte(j)
					if tc > 223 then tc=127 elseif tc>127 then tc=1 end
					local ws=charScales[tc]*fsize
					if tc>32 then
						glRectText(tc,0,lx,ly,ws,fsize)
					end
					lx=lx+ws
				end
				xx=xx-oo
				oo=0.0
				ls=i-st
				ds=st+1
				lx=bx
				ly=ly-fsize
			else
				ls=ls+1
			end
		end
		for j=ds,#str,1 do
			local tc=str:byte(j)
			if tc > 223 then tc=127 elseif tc>127 then tc=1 end
			local ws=charScales[tc]*fsize
			if tc>32 then
				glRectText(tc,0,lx,ly,ws,fsize)
			end
			lx=lx+ws
		end
	end
end
function spriteSize(name)
	local tex=Texture[name]
	
	if not tex then tex={} end
	if not tex.t then
		local ww,hh,tid=loadTexture(name)
		Texture[name]={t=tid,w=ww,h=hh}
		--print(ww,hh)
	end
	tex=Texture[name]
	--print(tex.w,tex.h)
	return tex.w,tex.h
end
function smooth()end
function noSmooth()end
eMode=0
function ellipseMode(mode)eMode=mode end

function pushStyle()end
function popStyle()end
rMode=0
function rectMode(mode)rMode=mode end
function noStroke()

end
function strokeWidth(width)
sWidth=width
end
function rect(x,y,w,h)
    if context then return end
		if rMode == CORNER then
			x=x+w*0.5
			y=y+h*0.5
		elseif rMode == CORNERS then
			w=w-x
			h=h-y
			x=x+w*0.5
			y=y+h*0.5
		end
		glRect(sWidth,x,y,w,h)
end

function readLocalData(path)
	if not datas then
	datas={}
	end
	if datas[path] then
		return datas[path]
	else
		local file = io.open((INTERNALDATAPATH or '.')..'/game-data/'..path, "r")
		if file then
			local content=file:read("*all")
			io.close(file)
			if content:sub(1,1)=='#' then
				datas[path]=tonumber(content:sub(2)) or 0
			else
				datas[path]=content
			end
			return datas[path]
		end
	end
end
function saveLocalData(path,value)
	datas[path]=value
	local gd = io.open((INTERNALDATAPATH or '.')..'/game-data','rb')
	if nil ~= gd then io.close(gd) else os.execute('mkdir -p '..(INTERNALDATAPATH or '.')..'/game-data') end
	local file = io.open((INTERNALDATAPATH or '.')..'/game-data/'..path, "w+b")
	if file then
		if 'number'==type(value) then
		file:write('#'..value)
		else
		file:write(value)
		end
		io.close(file)
	end
end
running=true
function openURL(addr)
	running=nil
end
mesh=class()
function mesh:addRect(x,y,w,h,angle)
	if not self.m then self.m={} end
	self.m[#self.m + 1] = {x,y,w,h,ang=math.deg(angle)}
	return #self.m
	--print("size of mesh is ".. #self.m)
end
function mesh:setRect(id,x,y,w,h,angle)
	if not self.m then self.m={} end
	if id and x and y and w and h then
		--print("setRect is ".. id)
		if id > #self.m then
		self.m[id]={x,y,w,h,ang=math.deg(angle)}
		else
		local mm=self.m[id]
		mm[1]=x
		mm[2]=y
		mm[3]=w
		mm[4]=h
		if angle then
			mm.ang=math.deg(angle)
		end
		end
	end
end
function mesh:setRectTex(id,x,y,w,h)
	if not self.m then self.m={} end
	if id and x and y and w and h then
		--print("setRect is ".. id)
		if id <= #self.m then
			local mm=self.m[id].offset
			if mm then
				mm.x=x
				mm.y=y
			else
				self.m[id].offset={x=x,y=y}
			end
		end
	end
	RUNINFO='('..x..','..y..','..w..','..h..')'
end
Texture={}
function mesh:draw(m)
	if context then return end
	local tid
	local ww
	local hh
	--if m then print("m:draw->"..m) else print("m:draw") end
	for i,vv in ipairs(self.m) do
		--print(i,v)
		if self.texture.tex then
			--vv.color=self.texture.fill
			tid=self.texture.tex
		else
			local tex=Texture[self.texture]
			if not tex then tex={} end
			tid=tex.t
			if not tid then
				ww,hh,tid=loadTexture(self.texture)
				Texture[self.texture]={t=tid,w=ww,h=hh}
			end
			end
			if vv.color then
				--print(v.color[1],v.color[2],v.color[3],v.color[4])
				tint(vv.color.r,vv.color.g,vv.color.b,vv.color.a)
				--print("Shadow",vv.color[1],vv.color[2],vv.color[3],vv.color[4])
			end
			pushMatrix()
			translate(vv[1],vv[2])
			rotate(vv.ang)
			if textureOffset and vv.offset then
				textureOffset(vv.offset.x,vv.offset.y)
			end
			gldraw(tid,0,0,vv[3],vv[4])
			-- glRect(1.0,0,0,vv[3],vv[4])
			popMatrix()
			--ellipse(vv[1],vv[2],15)
	end
	--print("mesh.texture = ",self.texture)
end
--[[function printcolor(c)
	if pColor then
	print("Shadow",c[1]..","..c[2]..","..c[3]..","..c[4])
	end
end]]
function mesh:setRectColor(id,cl)
	self.m[id].color=cl
end
image=class()
function image:init(w,h)
	self.width=w
	self.height=h
	--error("image.init("..w..","..h..")")
end
vec2=class()
function vec2:init(x,y)
	self.x=x
	self.y=y
	self.rotate=vec2.rotate
end
vec2.mt={}

setmetatable(vec2,vec2.mt)

vec2.mt.__div=function (a,b)
    local res = vec2(0,0)
	res.x = a.x / b
	res.y = a.y / b
    return res
end
vec2.mt.__add=function (a,b)
    local res = vec2(0,0)
	res.x = a.x + b.x
	res.y = a.y + b.y
    return res
end
vec2.mt.__sub=function (a,b)
    local res = vec2(0,0)
	res.x = a.x - b.x
	res.y = a.y - b.y
    return res
end
vec2.mt.__mul=function (a,b)
    local res = vec2(0,0)
	res.x = a.x * b
	res.y = a.y * b
    return res
end
vec2.mt.__call = function(class_tbl, ...)
   local obj = {}
   setmetatable(obj,vec2.mt)
   if class_tbl.init then
      class_tbl.init(obj,...)
   end
   return obj
end
function vec2:print()
	print("{"..self.x..","..self.y.."}")

end
function vec2:rotate(ang)
	local s= math.sin(ang)
	local c= math.cos(ang)
	return vec2(c*self.x-s*self.y,c*self.y+s*self.x)
end
if vec2.mt.__call then
v1=vec2(1,2)
v2=vec2(3,4)
v3=v1+v2
v3=v3:rotate(1)
vec2.print(v3)
end
