#include <Box2D/Box2D.h>
#include <vector>
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include "sound.h"
#define BIND_TO_LUA
#define registerFunc(func)      \
	lua_getglobal(L, #func);      \
	if (lua_isnil(L, -1)) {       \
		lua_pushcfunction(L, func); \
		lua_setglobal(L, #func);    \
	}                             \
	lua_pop(L, 1)
extern "C" int test(lua_State *L) {
	lua_pushinteger(L, 123);
	lua_pushinteger(L, 456);
	return 2;
}
#define SCALE(x) 0.03125 * x
#define REAL(x) 32 * x
int __index(lua_State *L) {
	lua_getmetatable(L, -2);
	lua_pushvalue(L, -2);
	lua_rawget(L, -2);
	if (lua_isfunction(L, -1)) {
		lua_remove(L, -2);
		return 1;
	} else
		lua_pop(L, 2);
	// printf("%s: %s\t%p\n", __FUNCTION__, lua_tostring(L, -1), lua_topointer(L, -2));
	lua_pushstring(L, "usr");
	lua_rawget(L, -3);
	// lua_getfield(L, -2, "usr");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return 0;
	} else {
		auto b = ((b2Body *)lua_touserdata(L, -1));
		// auto &o = b->GetPosition();
		lua_pop(L, 1);
		if (lua_isstring(L, -1)) {
			auto name = lua_tostring(L, -1);
			switch (name[0]) {
			case 't':
				lua_pushinteger(L, b->GetType());
				break;
			case 'x':
				lua_pushnumber(L, REAL(b->GetPosition().x));
				break;
			case 'y':
				lua_pushnumber(L, REAL(b->GetPosition().y));
				break;
			case 'a':
				lua_pushnumber(L, ('e' == name[4] ? b->GetAngle() : b->GetAngularVelocity()) * 180 / b2_pi);
				break;
			case 'l': {
				auto &v = b->GetLinearVelocity();
				lua_newtable(L);
				lua_pushnumber(L, REAL(v.x));
				lua_setfield(L, -2, "x");
				lua_pushnumber(L, REAL(v.y));
				lua_setfield(L, -2, "y");
			} break;
			case 'p': {
				auto &o = b->GetPosition();
				lua_newtable(L);
				lua_pushnumber(L, REAL(o.x));
				lua_setfield(L, -2, "x");
				lua_pushnumber(L, REAL(o.y));
				lua_setfield(L, -2, "y");
				auto c = b->GetWorldCenter();
				if (b2_staticBody == b->GetType()) {
					auto f = b->GetFixtureList();
					while (f) {
						auto s = f->GetShape();
						if (s->GetType() == b2Shape::e_polygon) {
							auto p = (b2PolygonShape *)s;
							auto v = p->m_vertices;
							auto cnt = p->GetVertexCount();
							b2Vec2 tmp = b2Vec2_zero;
							for (int i = cnt - 1; i >= 0; --i) {
								tmp += v[i];
							}
							tmp *= 1.0f / cnt;
							c = b->GetWorldPoint(tmp);
						}
						f = f->GetNext();
					}
				}
				lua_pushnumber(L, REAL(c.x));
				lua_setfield(L, -2, "cx");
				lua_pushnumber(L, REAL(c.y));
				lua_setfield(L, -2, "cy");
				lua_pushnumber(L, b->GetAngle());
				lua_setfield(L, -2, "angle");
			} break;
			case 'r': {
				auto f = b->GetFixtureList();
				lua_Number r;
				lua_Integer cnt = 0;
				while (f) {
					r += f->GetRestitution();
					++cnt;
					f = f->GetNext();
				}
				if (cnt > 1)
					r /= cnt;
				lua_pushnumber(L, r);
			} break;
			case 'f': {
				auto f = b->GetFixtureList();
				lua_Number r;
				lua_Integer cnt = 0;
				while (f) {
					r += f->GetFriction();
					++cnt;
					f = f->GetNext();
				}
				if (cnt > 1)
					r /= cnt;
				lua_pushnumber(L, r);
			} break;
			default:
				return 0;
			}
		} else
			return 0;
	}
	return 1;
}
int __newindex(lua_State *L) {
	// printf("%s: %s\t%p\n", __FUNCTION__, lua_tostring(L, -2), lua_topointer(L, -3));
	lua_pushstring(L, "usr");
	lua_rawget(L, -4);
	// lua_getfield(L, -3, "usr");
	/*
		self.points={}
		self.angle=0
		self.linearVelocity=vec2(0,0)
		self.angularVelocity=0
		self.x=0
		self.y=0
		self.radius=0
		self.shapeType=type_
		self.type=DYNAMIC
		body.restitution
		body.friction
	*/
	if (!lua_isnil(L, -1)) {
		auto o = (b2Body *)lua_touserdata(L, -1);
		auto t = o->GetPosition();
		auto angle = o->GetAngle();
		lua_pop(L, 1);
		auto name = lua_tostring(L, -2);
		// auto v = lua_tonumber(L, -1);
		if (lua_isstring(L, -2)) {
			switch (name[0]) {
			case 't':
				switch (lua_tointeger(L, -1)) {
				case 0:
					o->SetType(b2_staticBody);
					break;
				case 1:
					o->SetType(b2_kinematicBody);
					break;
				case 2:
					o->SetType(b2_dynamicBody);
					break;
				}
				break;
			case 'x':
				t.x = SCALE(lua_tonumber(L, -1));
				break;
			case 'y':
				t.y = SCALE(lua_tonumber(L, -1));
				break;
			case 'a':
				if ('e' == name[4]) {
					angle = lua_tonumber(L, -1) * b2_pi / 180;
					break;
				} else {
					o->SetAngularVelocity(lua_tonumber(L, -1) * b2_pi / 180);
					return 0;
				}
			case 'p':
				lua_getfield(L, -1, "x");
				if (lua_isnumber(L, -1))
					t.x = SCALE(lua_tonumber(L, -1));
				lua_getfield(L, -2, "y");
				if (lua_isnumber(L, -1))
					t.y = SCALE(lua_tonumber(L, -1));
				lua_getfield(L, -3, "angle");
				if (lua_isnumber(L, -1))
					angle = lua_tonumber(L, -1);
				lua_pop(L, 3);
				break;
			case 'l':
				lua_getfield(L, -1, "x");
				if (lua_isnumber(L, -1))
					t.x = SCALE(lua_tonumber(L, -1));
				lua_getfield(L, -2, "y");
				if (lua_isnumber(L, -1))
					t.y = SCALE(lua_tonumber(L, -1));
				o->SetLinearVelocity(t);
				return 0;
			case 'r': {
				auto f = o->GetFixtureList();
				auto fr = lua_tonumber(L, -1);
				while (f) {
					f->SetRestitution(fr);
					f = f->GetNext();
				}
			} break;
			case 'f': {
				auto f = o->GetFixtureList();
				auto fr = lua_tonumber(L, -1);
				while (f) {
					f->SetFriction(fr);
					f = f->GetNext();
				}
			} break;
			}
		}
		o->SetTransform(t, angle);
	} else
		lua_pop(L, 1);
	return 0;
}
int init(lua_State *L) { return 0; }
int applyForce(lua_State *L) {
	auto top = -lua_gettop(L);
	lua_getfield(L, top, "usr");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}
	auto b = (b2Body *)lua_touserdata(L, -1);
	lua_getfield(L, top, "x");
	lua_getfield(L, top - 1, "y");
	b2Vec2 force(SCALE(lua_tonumber(L, -2)), SCALE(lua_tonumber(L, -1)));
	if (top < -2) {
		lua_getfield(L, top - 1, "x");
		lua_getfield(L, top - 2, "y");
		b2Vec2 point(SCALE(lua_tonumber(L, -2)), SCALE(lua_tonumber(L, -1)));
		lua_pop(L, 5);
		b->ApplyForce(force, point, true);
	} else {
		lua_pop(L, 3);
		b->ApplyForceToCenter(force, true);
	}
	return 0;
}
int destroy(lua_State *L) {
	lua_getfield(L, -1, "usr");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}
	auto b = (b2Body *)lua_touserdata(L, -1);
	auto w = b->GetWorld();
#ifdef BIND_TO_LUA
	{
		lua_pushlightuserdata(L, w);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushlightuserdata(L, b);
		lua_pushnil(L);
		lua_rawset(L, -3);
		lua_pop(L, 1);
	}
#endif
	w->DestroyBody(b);
	lua_pop(L, 1);
	lua_pushnil(L);
	lua_setfield(L, -2, "usr");
	return 0;
}
extern "C" void stepWorld(lua_State *L, b2World *w);
float saveTime = 0;
int step(lua_State *L) {
	lua_getglobal(L, "physics");
	lua_getfield(L, -1, "running");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 3);
		return 0;
	}
	lua_getfield(L, -2, "usr");
	auto w = (b2World *)lua_touserdata(L, -1);
	lua_getglobal(L, "DeltaTime");
	float32 DeltaTime = lua_tonumber(L, -1);
	lua_pop(L, 4);
	saveTime += DeltaTime;
	if (saveTime > 0.0625f)
		saveTime = 0.0625f;
	while (saveTime >= 1.0f / 60) {
		w->Step(1.0f / 60, 10, 8);
		saveTime -= 1.0f / 60;
		stepWorld(L, w);
	}
	return 0;
}
int gravity(lua_State *L /*vec*/) {
	if (0 == lua_gettop(L)) {
		lua_getglobal(L, "physics");
		lua_getfield(L, -1, "usr");
		auto w = (b2World *)lua_touserdata(L, -1);
		lua_pop(L, 2);
		char info[32];
		auto g = w->GetGravity();
		sprintf(info, "gravity{x=%f,y=%f}", REAL(g.x), REAL(g.y));
		lua_pushstring(L, info);
		return 1;
	}
	lua_getfield(L, -1, "x");
	lua_getfield(L, -2, "y");
	auto x = lua_tonumber(L, -2);
	auto y = lua_tonumber(L, -1);
	lua_getglobal(L, "physics");
	lua_getfield(L, -1, "usr");
	auto w = (b2World *)lua_touserdata(L, -1);
	lua_pop(L, 4);
	w->SetGravity(b2Vec2(SCALE(x), SCALE(y)));
	return 0;
}
int resume(lua_State *L) {
	lua_getglobal(L, "physics");
	lua_getfield(L, -1, "running");
	if (lua_isnil(L, -1)) {
		lua_pushboolean(L, true);
		lua_setfield(L, -3, "running");
	}
	lua_pop(L, 2);
	return 0;
}
int pause(lua_State *L) {
	lua_getglobal(L, "physics");
	lua_getfield(L, -1, "running");
	if (!lua_isnil(L, -1)) {
		lua_pushnil(L);
		lua_setfield(L, -3, "running");
	}
	lua_pop(L, 2);
	return 0;
}
int body(lua_State *L /*tp,v1,v2,v3,v4*/) {
	auto top = lua_gettop(L);
	lua_getglobal(L, "physics");
	lua_getfield(L, -1, "usr");
	auto w = (b2World *)lua_touserdata(L, -1);
	b2BodyDef def;
	// def.allowSleep = false;
	def.type = b2_dynamicBody;
	auto b = w->CreateBody(&def);
	lua_pop(L, 2);
	auto type = lua_tointeger(L, -top);
	lua_newtable(L);
	lua_pushinteger(L, type);
	lua_setfield(L, -2, "shapeType");
	if (type == 3) {
		double max_x = FLT_MIN, max_y = FLT_MIN;
		double min_x = FLT_MAX, min_y = FLT_MAX;
		std::vector<b2Vec2> points;
		b2Vec2 center;
		center.x = 0;
		center.y = 0;
		for (int i = -top; i < -1; ++i) {
			auto tb = lua_istable(L, i);
			if (tb) {
				lua_getfield(L, i, "x");
				auto x = lua_tonumber(L, -1);
				max_x = b2Max(max_x, x);
				min_x = b2Min(min_x, x);
				lua_pop(L, 1);
				lua_getfield(L, i, "y");
				auto y = lua_tonumber(L, -1);
				max_y = b2Max(max_y, y);
				min_y = b2Min(min_y, y);
				lua_pop(L, 1);
				points.push_back({(float)x, (float)y});
				center.x += x;
				center.y += y;
			}
		}
		float scale = SCALE(1.0f) / points.size();
		float cx = center.x * scale;
		float cy = center.y * scale;
		lua_newtable(L);
		lua_Integer id = 0;
		for (auto &p : points) {
			/*p.x -= cx;
			p.y -= cy;*/
			lua_pushinteger(L, ++id);
			lua_newtable(L);
			lua_pushnumber(L, p.x);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, p.y);
			lua_setfield(L, -2, "y");
			lua_settable(L, -3);
		}
		lua_setfield(L, -2, "points");
		if (top == 3) {
			b2EdgeShape shape;
			lua_getfield(L, -2, "x");
			auto x = lua_tonumber(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -2, "y");
			auto y = lua_tonumber(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "x");
			auto xx = lua_tonumber(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "y");
			auto yy = lua_tonumber(L, -1);
			lua_pop(L, 1);
			shape.Set(SCALE(b2Vec2(x, y)), SCALE(b2Vec2(x, y)));
			b->CreateFixture(&shape, 0.0f);
			b->SetType(b2_staticBody);
		} else if (top > 3) {
			b2PolygonShape shape;
			// shape.SetAsBox((max_x - min_x) * SCALE(0.5), (max_y - min_y) * SCALE(0.5), b2Vec2(cx, cy), 0);
			for (auto &p : points)
				p = SCALE(p);
			shape.Set(points.data(), points.size());
			b->CreateFixture(&shape, 1.0f);
			/*lua_pushnumber(L, max_x - min_x);
			lua_setfield(L, -2, "width");
			lua_pushnumber(L, max_y - min_y);
			lua_setfield(L, -2, "height");*/
		}
	} else {
		auto radius = lua_tonumber(L, -2);
		b2CircleShape shape;
		shape.m_radius = SCALE(radius);
		b->CreateFixture(&shape, 1.0f);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "radius");
	}
	// Body *n = (Body *)malloc(sizeof(Body));
	// n->x = n->y = n->z = 66;
	// uds.push_back(n);
	/*lua_pushinteger(L, b2_dynamicBody);
	lua_setfield(L, -2, "type");
	lua_pushnumber(L, 30);
	lua_setfield(L, -2, "angle");*/
	lua_pushlightuserdata(L, b);
	lua_setfield(L, -2, "usr");
	lua_getglobal(L, "Body");
	lua_setmetatable(L, -2);
#ifdef BIND_TO_LUA
	{
		lua_pushlightuserdata(L, w);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushlightuserdata(L, b);
		lua_pushvalue(L, -3);
		lua_rawset(L, -3);
		lua_pop(L, 1);
	}
#endif
	/*{
		lua_getglobal(L, "physics");
		lua_getfield(L, -1, "bodies");
		lua_pushinteger(L, w->GetBodyCount());
		lua_pushvalue(L, -3);
		lua_settable(L, -2);
		lua_pop(L, 2);
	}*/
	return 1;
}
extern "C" int drawLine(float w, float x, float y, float x1, float y1);
#include <vector>
enum STATE { UNKNOW, BEGIN, MOVING, END };
struct ContactPoint {
	b2Fixture *fixtureA;
	b2Fixture *fixtureB;
	int childIndexA;
	int childIndexB;
	STATE state;
	bool touched, dyn;
	int point;
	b2Vec2 position, normal;
	float32 impulse, speed;
	bool operator==(const ContactPoint &other) const { return (fixtureA == other.fixtureA) && (fixtureB == other.fixtureB) && (childIndexA == other.childIndexA) && (childIndexB == other.childIndexB); }
};
class Contact : public b2ContactListener {
	void BeginContact(b2Contact *contact) override {
		ContactPoint cp;
		cp.childIndexA = contact->GetChildIndexA();
		cp.childIndexB = contact->GetChildIndexB();
		cp.fixtureA = contact->GetFixtureA();
		cp.fixtureB = contact->GetFixtureB();
		auto ret = std::find(points.begin(), points.end(), cp);
		ContactPoint *ccp;
		if (ret == points.end()) {
			cp.state = BEGIN;
			cp.impulse = 0.0f;
			cp.touched = contact->IsTouching();
			points.push_back(cp);
			ccp = &points.back();
		} else {
			(*ret).state = BEGIN;
			(*ret).impulse = 0.0f;
			(*ret).touched = contact->IsTouching();
			ccp = &*ret;
		}
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);
		ccp->position.SetZero();
		ccp->point = contact->GetManifold()->pointCount;
		ccp->normal = worldManifold.normal;
		ccp->impulse = 0.0f;
		for (int i = 0; i < ccp->point; ++i) {
			ccp->position += worldManifold.points[i];
		}
		auto wp = 1.0f / ccp->point * ccp->position;
		auto a = cp.fixtureA->GetBody();
		auto b = cp.fixtureB->GetBody();
		ccp->dyn = a->GetType() == a->GetType();
		ccp->speed = (a->GetLinearVelocityFromWorldPoint(wp) - b->GetLinearVelocityFromWorldPoint(wp)).Length();
		ccp->position = 32.0f * wp;
	}
	void EndContact(b2Contact *contact) override {
		ContactPoint cp;
		cp.childIndexA = contact->GetChildIndexA();
		cp.childIndexB = contact->GetChildIndexB();
		cp.fixtureA = contact->GetFixtureA();
		cp.fixtureB = contact->GetFixtureB();
		auto ret = std::find(points.begin(), points.end(), cp);
		ContactPoint *ccp;
		if (ret == points.end()) {
			cp.state = END;
			cp.touched = contact->IsTouching();
			points.push_back(cp);
			ccp = &points.back();
		} else {
			(*ret).state = END;
			(*ret).touched = contact->IsTouching();
			ccp = &*ret;
		}
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);
		ccp->position.SetZero();
		ccp->point = contact->GetManifold()->pointCount;
		ccp->normal = worldManifold.normal;
		ccp->impulse = 0.0f;
		for (int i = 0; i < ccp->point; ++i) {
			ccp->position += worldManifold.points[i];
		}
		ccp->position *= 32.0f / ccp->point;
	}
	void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) override {
		ContactPoint cp;
		cp.childIndexA = contact->GetChildIndexA();
		cp.childIndexB = contact->GetChildIndexB();
		cp.fixtureA = contact->GetFixtureA();
		cp.fixtureB = contact->GetFixtureB();
		auto ret = std::find(points.begin(), points.end(), cp);
		ContactPoint *ccp;
		if (ret == points.end()) {
			cp.state = UNKNOW;
			points.push_back(cp);
			ccp = &points.back();
		} else {
			ccp = &*ret;
		}
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);
		ccp->position.SetZero();
		ccp->point = impulse->count;
		ccp->normal = worldManifold.normal;
		ccp->impulse = 0.0f;
		auto a = cp.fixtureA->GetBody();
		auto b = cp.fixtureB->GetBody();
		float32 mass = a->GetMass() + b->GetMass();
		if (ccp->dyn)
			mass *= 0.5f;
		for (int i = 0; i < ccp->point; ++i) {
			// if (impulse->normalImpulses[i] > b2Abs(impulse->tangentImpulses[i]))
			ccp->impulse += impulse->normalImpulses[i];
			ccp->position += worldManifold.points[i];
		}
		auto wp = 1.0f / ccp->point * ccp->position;
		// ccp->speed = (a->GetLinearVelocityFromWorldPoint(wp) - b->GetLinearVelocityFromWorldPoint(wp)).Length();
		ccp->position = 32.0f * wp;
		ccp->impulse /= ccp->point;
	}
	int skip = 0;
public:
	void post(lua_State *L, b2World *w) {
		if (false) {
			auto c = soundPoints.begin();
			while (c != soundPoints.end()) {
				c->z *= 0.95;
				if (c->z < 5) {
					c = soundPoints.erase(c);
				} else {
					++c;
				}
			}
		}
			// Sound_clear();
#ifdef BIND_TO_LUA
		if (true) {
			float normalImpulse = 0;
			auto c = points.begin();
			ContactPoint cur;
			cur.impulse = 0;
			while (c != points.end()) {
				if (BEGIN == c->state && c->impulse > cur.impulse)
					cur = *c;
				if (END == c->state)
					c = points.erase(c);
				else {
					if (BEGIN == c->state)
						c->state = MOVING;
					++c;
				}
			}
			if (cur.impulse > 0) {
				lua_getglobal(L, "collide");
				if (lua_isfunction(L, -1)) {
					auto a = cur.fixtureA->GetBody();
					auto b = cur.fixtureB->GetBody();
					lua_createtable(L, 0, 4);
					lua_pushliteral(L, "state");
					lua_pushinteger(L, cur.state);
					lua_rawset(L, -3);
					lua_pushliteral(L, "normalImpulse");
					lua_pushnumber(L, cur.impulse);
					lua_rawset(L, -3);
					lua_pushlightuserdata(L, w);
					lua_rawget(L, LUA_REGISTRYINDEX);
					lua_pushliteral(L, "bodyA");
					lua_pushlightuserdata(L, a);
					lua_rawget(L, -3);
					lua_rawset(L, -4);
					lua_pushliteral(L, "bodyB");
					lua_pushlightuserdata(L, b);
					lua_rawget(L, -3);
					lua_rawset(L, -4);
					lua_pop(L, 1);
#if 0
					if (c->state == BEGIN && normalImpulse < c->impulse) {
						normalImpulse = c->impulse;
						lua_pushvalue(L, -1);
						lua_setglobal(L, "Contact");
						luaL_dostring(L, R"lua(
SAVE_CODE='Contact '
for i,v in pairs(Contact) do
SAVE_CODE=SAVE_CODE..i..':'..tostring(v)..'\n'
end
)lua");
					}
#endif
					lua_call(L, 1, 0);
				} else
					lua_pop(L, 1);
			}
			return;
		}
#endif
		bool sound = false;
		float maxImpulse = 0;
		auto c = points.begin();
		b2Vec2 at;
		while (c != points.end()) {
			if (BEGIN == c->state) {
				if (c->touched && c->impulse > 2 && c->speed > 5) {
					if (c->impulse > maxImpulse) {
						maxImpulse = c->impulse;
						if (c->dyn)
							skip = 0;
						at = c->position;
					}
					sound = true;
				}
				/*lua_getglobal(L, "collide");
				lua_newtable(L);
				lua_pushvalue(L, 0);
				lua_setfield(L, -2, "bodyA");
				lua_pushvalue(L, 0);
				lua_setfield(L, -2, "bodyB");
				lua_call(L, 1, 0);*/
				c->state = MOVING;
			} else if (END == c->state) {
				c = points.erase(c);
				continue;
			}
			++c;
		}
		if (skip) {
			--skip;
		} else if (sound) {
			soundPoints.push_back(b2Vec3(at.x, at.y, 20));
			lua_pushnumber(L, maxImpulse);
			lua_setglobal(L, "IMPULSE");
			// luaL_dostring(L, "sound(DATA,'ZgJA6gBAQGhEO1QUdkp/PbB+pT6YmjY/WAAAaCVGOw0/FVV4',IMPULSE/20)");
			// luaL_dostring(L, "sound(DATA,'ZgFAegABbgJAWx5ZoXmyPTBczj52Sn89XQBHTzRFQEALfBkQ',IMPULSE/20)");
			luaL_dostring(L, "sound(DATA,'ZgNADQA9PUJCQAthAAAAALicKz7PfFA+fwBZRkA+QzM+PDtD',IMPULSE/20)");
			skip = 5;
		}
	}
	std::vector<ContactPoint> points;
	std::vector<b2Vec3> soundPoints;
};
Contact *listener = nullptr;
extern "C" void sound(lua_State *L);
extern "C" void stepWorld(lua_State *L, b2World *w) {
	if (nullptr == listener)
		return;
	listener->post(L, w);
}
extern "C" void drawPhysics(lua_State *L) {
	if (nullptr == listener)
		return;
	for (auto &c : listener->soundPoints) {
		drawLine(2.0f, c.x - c.z, c.y - c.z, c.x + c.z, c.y + c.z);
		drawLine(2.0f, c.x + c.z, c.y - c.z, c.x - c.z, c.y + c.z);
	}
	/*for (auto &c : listener->points) {
		if (c.touched)
			drawLine(c.impulse > 50 ? 5.0f : 1.0f, c.position.x, c.position.y, c.position.x + c.normal.x * c.impulse * 10, c.position.y + c.normal.y * c.impulse * 10);
	}*/
	{
		auto c = listener->soundPoints.begin();
		while (c != listener->soundPoints.end()) {
			c->z *= 0.95;
			if (c->z < 5) {
				c = listener->soundPoints.erase(c);
			} else {
				++c;
			}
		}
	}
}
extern "C" void initPhysicsLib(lua_State *L) {
	auto w = new b2World(b2Vec2_zero);
#ifdef BIND_TO_LUA
	{
		lua_pushlightuserdata(L, w);
		lua_newtable(L);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
#endif
	listener = new Contact();
	w->SetContactListener(listener);
	w->SetContinuousPhysics(true);
	// w->SetAllowSleeping(false);
	/*w->SetSubStepping(true);
	w->SetWarmStarting(true);*/
	lua_getglobal(L, "physics");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushinteger(L, 1);
		lua_setglobal(L, "POLYGON");
		lua_pushinteger(L, 0);
		lua_setglobal(L, "CIRCLE");
		lua_pushinteger(L, b2_dynamicBody);
		lua_setglobal(L, "DYNAMIC");
		lua_pushinteger(L, b2_staticBody);
		lua_setglobal(L, "STATIC");
		lua_pushinteger(L, b2_kinematicBody);
		lua_setglobal(L, "KINEMATIC");
		{
			lua_newtable(L);
			lua_pushcfunction(L, applyForce);
			lua_setfield(L, -2, "applyForce");
			lua_pushcfunction(L, destroy);
			lua_setfield(L, -2, "destroy");
			/*lua_pushcfunction(L, init);
			lua_setfield(L, -2, "init");*/
			lua_pushcfunction(L, __newindex);
			// lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__newindex");
			lua_pushcfunction(L, __index);
			// lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			//
			/*lua_newtable(L);
			lua_pushcfunction(L, __newindex);
			lua_setfield(L, -2, "__newindex");
			lua_pushcfunction(L, __index);
			lua_setfield(L, -2, "__index");
			lua_setmetatable(L, -2);*/
			lua_setglobal(L, "Body");
		}
		lua_newtable(L);
		{
			lua_newtable(L);
			{
				lua_pushinteger(L, 1);
				lua_pushstring(L, "Array");
				lua_settable(L, -3);
				lua_pushinteger(L, 2);
				lua_pushstring(L, "Second");
				lua_settable(L, -3);
			}
			lua_setfield(L, -2, "bodies");
		}
		lua_pushlightuserdata(L, w);
		lua_setfield(L, -2, "usr");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "stop");
		lua_pushcfunction(L, gravity);
		lua_setfield(L, -2, "gravity");
		lua_pushcfunction(L, resume);
		lua_setfield(L, -2, "resume");
		lua_pushcfunction(L, pause);
		lua_setfield(L, -2, "pause");
		lua_pushcfunction(L, body);
		lua_setfield(L, -2, "body");
		lua_pushcfunction(L, step);
		lua_setfield(L, -2, "step");
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "running");
		lua_setglobal(L, "physics");
	} else
		lua_pop(L, 1);
	registerFunc(test);
}
extern "C" void freePhysics(lua_State *L) {
	auto top = lua_gettop(L);
	lua_getglobal(L, "physics");
	lua_getfield(L, -1, "usr");
	auto w = (b2World *)lua_touserdata(L, -1);
#ifdef BIND_TO_LUA
	{
		lua_pushlightuserdata(L, w);
		lua_rawget(L, LUA_REGISTRYINDEX);
		auto b = w->GetBodyList();
		while (b) {
			lua_pushlightuserdata(L, b);
			lua_pushnil(L);
			lua_rawset(L, -3);
			b = b->GetNext();
		}
		lua_pop(L, 1);
		lua_pushlightuserdata(L, w);
		lua_pushnil(L);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
#endif
	lua_pushnil(L);
	lua_setfield(L, -3, "usr");
	lua_pop(L, 2);
	lua_pushnil(L);
	lua_setglobal(L, "physics");
	w->SetContactListener(nullptr);
	delete listener;
	delete w;
}
