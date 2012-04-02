// 3D World - collision detection/collision object classes
// by Frank Gennari
// 7/23/06

#ifndef _COLLISION_DETECT_H_
#define _COLLISION_DETECT_H_


#include "3DWorld.h"

typedef bool (*collision_func)(int, int, vector3d const &, point const &, float, int);

// object/collision object types/status
enum {COLL_NULL     = 0, COLL_CUBE,     COLL_CYLINDER, COLL_SPHERE,  COLL_CYLINDER_ROT, COLL_POLYGON,  COLL_INVALID};
enum {COLL_UNUSED   = 0, COLL_FREED,    COLL_PENDING,  COLL_STATIC,  COLL_DYNAMIC,      COLL_NEGATIVE, COLL_TO_REMOVE};
enum {OBJ_STAT_BAD  = 0, OBJ_STAT_AIR,  OBJ_STAT_COLL, OBJ_STAT_GND, OBJ_STAT_STOP,     OBJ_STAT_RES};
enum {COBJ_TYPE_STD = 0, COBJ_TYPE_MODEL3D, COBJ_TYPE_VOX_TERRAIN};

unsigned const OBJ_CNT_REM_TJ = 1;


class obj_layer { // size = 60

public:
	bool draw, shadow, swap_txy;
	unsigned char cobj_type;
	float elastic, tscale, specular, shine, tdx, tdy, refract_ix, light_atten;
	int tid;
	collision_func coll_func;
	colorRGBA color;

	obj_layer(float e=0.0, colorRGBA const &c=WHITE, bool d=0, const collision_func cf=NULL, int ti=-1,
		float ts=1.0, float spec=0.0, float shi=0.0) :
		draw(d), shadow(1), swap_txy(0), cobj_type(COBJ_TYPE_STD), elastic(e), tscale(ts), specular(spec), shine(shi),
		tdx(0.0), tdy(0.0), refract_ix(1.0), light_atten(0.0), tid(ti), coll_func(cf), color(c) {}

	// assumes obj_layer contained classes are POD with no padding
	bool operator==(obj_layer const &cobj) const {return (memcmp(this, &cobj, sizeof(obj_layer)) == 0);}
	bool operator< (obj_layer const &cobj) const {return (memcmp(this, &cobj, sizeof(obj_layer)) <  0);}
	bool no_draw()        const {return (!draw || color.alpha == 0.0);}
	bool has_alpha_texture() const;
	bool is_semi_trans()  const {return (color.alpha < 1.0 || has_alpha_texture());}
	bool might_be_drawn() const {return (draw || cobj_type != COBJ_TYPE_STD);}
	bool is_glass()       const {return (tid < 0 && color.alpha <= 0.5);}
};


class cobj_params : public obj_layer { // size = 68

public:
	int cf_index;
	unsigned char surfs;
	bool is_dynamic, is_destroyable, no_coll;
	//obj_layer *layer;

	cobj_params() : cf_index(-1), surfs(0), is_dynamic(0), is_destroyable(0), no_coll(0) {}
	cobj_params(float e, colorRGBA const &c, bool d, bool id, const collision_func cf=NULL, int ci=0,
		int ti=-1, float ts=1.0, int s=0, float spec=0.0, float shi=0.0, bool nc=0) :
		obj_layer(e, c, d, cf, ti, ts, spec, shi), cf_index(ci), surfs(s), is_dynamic(id), is_destroyable(0), no_coll(nc) {}
};


class coll_obj_group;
class csg_cube;


class coll_obj : public cube_t { // size = 224

public:
	cobj_params cp; // could store unique cps in a set of material properties to reduce memory requirements slightly
	char type, destroy, status;
	unsigned char last_coll, coll_type;
	bool fixed, is_billboard, falling;
	float radius, radius2, thickness, volume, v_fall;
	int counter, id;
	short platform_id, group_id, waypt_id, npoints;
	point points[N_COLL_POLY_PTS];
	vector3d norm, texture_offset;
	vector<int> occluders;

	coll_obj() : type(COLL_NULL), destroy(0), status(COLL_UNUSED), last_coll(0), coll_type(0), fixed(0), is_billboard(0),
		falling(0), radius(0.0), radius2(0.0), thickness(0.0), volume(0.0), v_fall(0.0), counter(0), id(-1), platform_id(-1),
		group_id(-1), waypt_id(-1), npoints(0), norm(zero_vector), texture_offset(zero_vector) {}
	void init();
	void clear_internal_data();
	void calc_size();
	float calc_min_dim() const;
	bool clip_in_2d(float const bb[2][2], float &ztop, int d1, int d2, int dir) const;
	void set_npoints();
	void set_from_pts(point const *const pts, unsigned npts);
	void print_bounds() const;
	void bb_union(float bb[3][2], int init);
	void draw_cobj(unsigned &cix, int &last_tid, int &last_group_id, shader_t *shader) const;
	int  simple_draw(int ndiv, int in_cur_prim=PRIM_DISABLED, bool no_normals=0, bool in_dlist=0) const;
	void add_to_vector(coll_obj_group &cobjs, int type_);
	void check_if_cube();
	void add_as_fixed_cobj();
	int  add_coll_cobj();
	void re_add_coll_cobj(int index, int remove_old=1, int dhcm=0);
	bool subdiv_fixed_cube(coll_obj_group &cobjs);
	bool subtract_from_cobj(coll_obj_group &new_cobjs, csg_cube const &cube, bool include_polys);
	int  intersects_cobj(coll_obj const &c, float toler=0.0) const;
	int  is_anchored() const;
	void shift_by(vector3d const &vd, bool force=0, bool no_texture_offset=0);
	void add_to_platform() const;
	bool dynamic_shadows_only() const;
	void add_shadow(unsigned light_sources, bool dynamic) const;
	bool cobj_plane_side_test(point const *pts, unsigned npts, point const &lpos) const;
	bool operator<(const coll_obj &cobj) const {return (volume < cobj.volume);} // sort by size
	bool equal_params(const coll_obj &c) const {return (type == c.type && status == c.status &&
		platform_id == c.platform_id && group_id == c.group_id && cp == c.cp);}
	bool no_draw()        const {return (status == COLL_UNUSED || status == COLL_FREED || cp.no_draw());}
	bool disabled()       const {return (status != COLL_DYNAMIC && status != COLL_STATIC);}
	bool no_collision()   const {return (disabled() || cp.no_coll);}
	bool is_semi_trans()  const {return cp.is_semi_trans();}
	bool freed_unused()   const {return (status == COLL_FREED || status == COLL_UNUSED);}
	bool is_occluder()    const;// {return (status == COLL_STATIC && type == COLL_CUBE && cp.draw && !is_semi_trans());}
	bool is_big_occluder()const {return (is_occluder() && fixed && volume > 0.001);}
	bool maybe_is_moving()const {return (platform_id >= 0 || falling);}
	bool is_player()      const;
	bool is_invis_player()const;
	bool truly_static()   const;
	bool no_shadow()      const {return (status != COLL_STATIC || !cp.shadow || cp.color.alpha < MIN_SHADOW_ALPHA || maybe_is_moving());}
	bool is_cylinder()    const {return (type == COLL_CYLINDER || type == COLL_CYLINDER_ROT);}
	bool is_thin_poly()   const {return (type == COLL_POLYGON && thickness <= MIN_POLY_THICK);}
	// allow destroyable and transparent objects, drawn or opaque model3d shapes
	bool can_be_scorched()const {return (status == COLL_STATIC && !cp.has_alpha_texture() && (!no_draw() || (cp.cobj_type != COBJ_TYPE_STD && cp.color.A == 1.0)));}
	point get_center_pt() const;
	float get_max_dim()   const;
	float get_light_transmit(point v1, point v2) const;
	colorRGBA get_avg_color() const;
	void bounding_sphere(point &center, float &brad) const;
	bool has_poly_billboard_alpha() const {return (is_billboard && is_thin_poly() && npoints == 4 && cp.has_alpha_texture());}
	bool check_poly_billboard_alpha(point const &p1, point const &p2, float t) const;
	bool line_intersect(point const &p1, point const &p2) const;
	bool line_int_exact(point const &p1, point const &p2, float &t, vector3d &cnorm, float tmin, float tmax) const;
	bool intersects_all_pts(point const &pos, point const *const pts, unsigned npts) const; // coll_cell_search.cpp
	bool is_occluded_from_camera() const;
	void register_coll(unsigned char coll_time, unsigned char coll_type_) {last_coll = coll_time; coll_type = coll_type_;}
	void create_portal() const; // destroy_cobj.cpp
	void add_connect_waypoint(); // waypoints.cpp
	void remove_waypoint();

	// drawing code
	void draw_coll_cube(int do_fill, int tid, shader_t *shader) const;
	void set_poly_texgen(int tid, vector3d const &normal, shader_t *shader) const;
	void draw_polygon(int tid, point const *points, int npoints, vector3d const &normal, shader_t *shader) const;
	void draw_extruded_polygon(int tid, shader_t *shader) const;
};


struct cobj_id_set_t : public set<unsigned> {

	void must_insert(unsigned index) {
		bool const did_ins(insert(index).second);
		assert(did_ins);
	}
	void must_erase(unsigned index) {
		unsigned const num_removed(erase(index));
		assert(num_removed == 1);
	}
	bool has(unsigned index) {return find(index) != end();}
};


class coll_obj_group : public vector<coll_obj> {

public:
	bool has_lt_atten;
	cobj_id_set_t dynamic_ids, drawn_ids, platform_ids;

	coll_obj_group() : has_lt_atten(0) {}
	void clear();
	void finalize();
	void remove_invalid_cobjs();
	void check_cubes();
	void merge_cubes();
	void process_negative_shapes();
	void remove_overlapping_cubes();
	void subdiv_cubes();
	void sort_cobjs_for_rendering();
	void set_coll_obj_props(int index, int type, float radius, float radius2, int platform_id, cobj_params const &cparams);
};


struct cobj_query_callback {
	virtual void register_cobj(coll_obj const &cobj) = 0;
};


class polygon_t : public vector<vert_norm_tc> {

public:
	colorRGBA color;

	polygon_t(colorRGBA const &c=ALPHA0) : color(c) {}
	polygon_t(vector<vert_norm_tc> const &vv, colorRGBA const &c=ALPHA0) : vector<vert_norm_tc>(vv), color(c) {}
	polygon_t(triangle const &t, colorRGBA const &c=WHITE) : color(c) {from_triangle(t);}
	void from_triangle(triangle const &t);
	bool is_convex() const;
	bool is_coplanar(float thresh) const;
	vector3d get_planar_normal() const;
	bool is_valid() const {return (size() >= 3 && is_triangle_valid((*this)[0].v, (*this)[1].v, (*this)[2].v));}
	void from_points(vector<point> const &pts);
};


struct coll_tquad : public tquad_t { // size = 68

	vector3d normal;
	
	union {
		unsigned cid;
		color_wrapper color;
	};
	coll_tquad() {}
	coll_tquad(coll_obj const &c);
	coll_tquad(polygon_t const &p);
	coll_tquad(triangle const &t, colorRGBA const &c=WHITE);

	static bool is_cobj_valid(coll_obj const &c) {
		return (!c.disabled() && c.type == COLL_POLYGON && (c.npoints == 3 || c.npoints == 4) && c.thickness <= MIN_POLY_THICK);
	}
	bool line_intersect(point const &p1, point const &p2) const {
		float t;
		return line_poly_intersect(p1, p2, pts, npts, normal, t);
	}
	bool line_int_exact(point const &p1, point const &p2, float &t, vector3d &cnorm, float tmin, float tmax) const {
		if (!line_poly_intersect(p1, p2, pts, npts, normal, t) || t > tmax || t < tmin) return 0;
		cnorm = get_poly_dir_norm(normal, p1, (p2 - p1), t);
		return 1;
	}
};


void copy_polygon_to_cobj(polygon_t const &poly, coll_obj &cobj);
void copy_tquad_to_cobj(coll_tquad const &tquad, coll_obj &cobj);


struct coll_cell { // size = 52

	float zmin, zmax, occ_zmin, occ_zmax;
	vector<int> cvals;

	void clear(bool clear_vectors);
	void update_zmm(float zmin_, float zmax_, coll_obj const &cobj);
	
	void add_entry(int index) {
		if (INIT_CCELL_SIZE > 0 && cvals.capacity() == 0) cvals.reserve(INIT_CCELL_SIZE);
		cvals.push_back(index);
	}
};


struct color_tid_vol : public cube_t {

	int cid, tid, destroy;
	bool draw, unanchored, is_2d;
	float volume, thickness, tscale;
	colorRGBA color;
	color_tid_vol(coll_obj const &cobj, float volume_, float thickness_, bool ua);
};


class platform { // animated (player controlled) scene object

	// constants
	bool const cont; // continuous - always in motion
	float const fspeed, rspeed; // velocity of forward/reverse motion in units per tick (can be negative)
	float const sdelay, rdelay; // start delay / reverse delay in ticks
	float const ext_dist, act_dist; // distance traveled, activation distance
	point origin; // initial position (moved by shifting)
	vector3d const dir; // direction of motion (travels to dir*dist)

	// state variables
	enum {ST_NOACT=0, ST_WAIT, ST_FWD, ST_CHDIR, ST_REV};
	int state; // 0 = not activated, 1 = activated but waiting, 2 = forward, 3 = waiting to go back, 4 = back
	float ns_time; // time to next state in ticks (can be negative if frame time is larger than a delay/travel time)
	point pos; // current position - dist is calculated from this point (delta = pos-origin)
	vector3d delta; // last change in position

public:
	// other data
	vector<unsigned> cobjs; // collision object(s) bound to this platform
	
	platform(float fs=1.0, float rs=1.0, float sd=0.0, float rd=0.0, float dst=1.0, float ad=0.0,
		point const &o=all_zeros, vector3d const &dir_=plus_z, bool c=0);
	bool has_dynamic_shadows() const {return (cont || state >= ST_FWD);}
	vector3d get_delta()       const {return (pos - origin);}
	vector3d get_range()       const {return dir*ext_dist;}
	vector3d get_last_delta()  const {return delta;}
	vector3d get_velocity()    const;
	void clear_last_delta() {delta = all_zeros;}
	void add_cobj(unsigned cobj);
	void next_frame();
	void shift_by(vector3d const &val);
	void reset();
	void activate();
	bool check_activate(point const &p, float radius);
	void advance_timestep();
	bool is_moving() const {return (state == ST_FWD || state == ST_REV);}
};


struct platform_cont : public deque<platform> {

	bool add_from_file(FILE *fp);
	void check_activate(point const &p, float radius);
	void shift_by(vector3d const &val);
	void advance_timestep();
};


class shadow_sphere : public sphere_t {

public:
	char ctype;
	int cid;

	shadow_sphere() : ctype(COLL_NULL), cid(-1) {}
	shadow_sphere(point const &pos0, float radius0, int cid0);
	
	inline bool line_intersect(point const &p1, point const &p2) const {
		return (line_sphere_intersect(p1, p2, pos, radius) && (ctype == COLL_SPHERE || line_intersect_cobj(p1, p2)));
	}
	bool line_intersect_cobj(point const &p1, point const &p2) const;

	inline bool test_volume(point const *const pts, unsigned npts, point const &lpos) const {
		return (cid < 0 || test_volume_cobj(pts, npts, lpos));
	}
	bool test_volume_cobj(point const *const pts, unsigned npts, point const &lpos) const;
};


class obj_draw_group {

	unsigned start_cix, end_cix, dlist;
	bool use_dlist, inside_beg_end, creating_new_dlist;

public:
	obj_draw_group(bool use_dlist_=0) : start_cix(0), end_cix(0), dlist(0), use_dlist(use_dlist_), inside_beg_end(0), creating_new_dlist(0) {}
	void create_dlist();
	void free_dlist();
	void begin_render(unsigned &cix);
	void end_render();
	void mark_as_drawn(unsigned cix);
	void set_dlist_enable(bool en) {assert(dlist == 0); use_dlist = en;} // can't call while the dlist is valid
	bool in_dlist   () const {return creating_new_dlist;}
	bool skip_render() const {return (in_dlist() && !creating_new_dlist);}
};


#endif // _COLLISION_DETECT_H_

