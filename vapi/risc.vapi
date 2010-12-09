[CCode (cheader_filename = "renderer.h")]
[CCode (cheader_filename = "game.h")]
[CCode (cheader_filename = "particle.h")]
[CCode (cheader_filename = "glutil.h")]
[CCode (cheader_filename = "util.h")]
[CCode (cheader_filename = "team.h")]
[CCode (cheader_filename = "physics.h")]
[CCode (cheader_filename = "bullet.h")]
[CCode (cheader_filename = "ship.h")]

namespace RISC {
	[CCode (cname = "all_bullets")]
	public GLib.List<Bullet> all_bullets;
	[CCode (cname = "bullet_hits")]
	public GLib.List<BulletHit> bullet_hits;

	namespace GL13 {
    [CCode (cname = "init_gl13")]
    public void init();
    [CCode (cname = "reset_gl13")]
    public void reset();
    [CCode (cname = "render_gl13")]
    public void render(bool paused, bool render_all_debug_lines);
    [CCode (cname = "reshape_gl13")]
    public void reshape(int x, int y);
		[CCode (cname = "glutil_vprintf")]
		public void vprintf(int x, int y, string fmt, va_list ap);
		[CCode (cname = "glColor32")]
		void glColor32(uint32 c);
		[CCode (cname = "zoom")]
		public int zoom(int x, int y, double force);
		[CCode (cname = "pick")]
		public void pick(int x, int y);
    [CCode (cname = "emit_particles")]
    public void emit_particles();
    [CCode (cname = "S")]
		public Vector.Vec2 S(Vector.Vec2 p);
	}

		[CCode (cname = "game_init")]
		public int game_init(int seed, string scenario, string[] ais);
		[CCode (cname = "game_purge")]
		public void game_purge();
		[CCode (cname = "game_tick")]
		public void game_tick(double tick_length);
		[CCode (cname = "game_shutdown")]
		public void game_shutdown();

		[CCode (cname = "particle_tick")]
		public void particle_tick();

		[CCode (cname = "screenshot")]
		public void screenshot(string filename);
		
		[CCode (cname = "find_data_dir")]
		public bool find_data_dir();
		[CCode (cname = "data_path")]
		public string data_path(string subpath);

    [CCode (cname = "struct team", destroy_function = "")]
		[Compact]
		public class Team {
			public uint32 color;
			public string name;
			public string filename;
			public int ships;
		}

    [CCode (cname = "struct physics", destroy_function = "")]
		[Compact]
		public class Physics {
			public Vector.Vec2 p;
			public Vector.Vec2 p0;
			public Vector.Vec2 v;
			public Vector.Vec2 thrust;
			public double a;
			public double av;
			public double r;
			public double m;
		}

		public enum BulletType {
			[CCode (cname = "BULLET_SLUG")]
			SLUG,
			[CCode (cname = "BULLET_PLASMA")]
			PLASMA,
		}

    [CCode (cname = "struct bullet", destroy_function = "")]
		[Compact]
		public class Bullet {
			public Physics physics;
			public unowned Team team;
			public double ttl;
			public bool dead;
			public BulletType type;
		}

		public enum ParticleType {
			[CCode (cname = "PARTICLE_HIT")]
			HIT,
			[CCode (cname = "PARTICLE_PLASMA")]
			PLASMA,
			[CCode (cname = "PARTICLE_ENGINE")]
			ENGINE,
		}

    [CCode (cname = "struct particle", destroy_function = "")]
		[Compact]
		public class Particle {
			public Vector.Vec2 p;
			public Vector.Vec2 v;
			public uint16 ticks_left;
			public ParticleType type;

			[CCode (cname = "particle_shower")]
			public static void shower(ParticleType type,
			                          Vector.Vec2 p0, Vector.Vec2 v0, Vector.Vec2 v,
			                          double s_max, uint16 life_min, uint16 life_max,
			                          uint16 count);
		}

    [CCode (cname = "struct bullet_hit", destroy_function = "")]
		[Compact]
		public class BulletHit {
			public unowned Ship s;
			public unowned Bullet b;
			public Vector.Vec2 cp;
			public double e;
		}

		// XXX INCOMPLETE
    [CCode (cname = "struct ship", destroy_function = "")]
		[Compact]
		public class Ship {
			public uint32 api_id;
			//const struct ship_class *class;
			public unowned Team team;
			public Physics physics;
			public double energy;
			public double hull;
/*
			lua_State *lua, *global_lua;
			struct {
				lua_Alloc allocator;
				void *allocator_ud;
				int cur, limit;
			} mem;
*/
			public GLib.Rand prng;
			public bool dead;
			public bool ai_dead;
			//Vector.Vec2 tail[TAIL_SEGMENTS];
			public int tail_head;
			public int last_shot_tick;
			//GQueue *mq;
			public uint64 line_start_time;
			/*
			char line_info[256];
			struct {
				int num_lines;
				struct {
					Vec2 a, b;
				} lines[MAX_DEBUG_LINES];
			} debug;
			struct {
				const struct gfx_class *class;
				double angle;
			} gfx;
			*/
		}

		[CCode (cname = "game_check_victory")]
		public unowned Team game_check_victory();
}
