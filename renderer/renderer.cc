#include "renderer/renderer.h"
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <stdint.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include <Box2D/Box2D.h>

#include "sim/ship.h"
#include "sim/ship_class.h"
#include "sim/bullet.h"
#include "sim/beam.h"
#include "sim/team.h"
#include "sim/model.h"
#include "sim/math_util.h"
#include "common/log.h"
#include "common/resources.h"
#include "gl/program.h"
#include "gl/shader.h"
#include "gl/buffer.h"
#include "gl/texture.h"
#include "gl/check.h"
#include "renderer/font.h"
#include "renderer/batches/ship.h"
#include "renderer/batches/tail.h"

using glm::vec2;
using glm::vec4;
using std::make_shared;
using std::shared_ptr;
using boost::scoped_ptr;
using namespace Oort::RendererBatches;

namespace Oort {

Renderer::Renderer(shared_ptr<Game> game)
  : game(game),
    bullet_prog(new GL::Program(
      make_shared<GL::VertexShader>(load_resource("shaders/bullet.v.glsl")),
      make_shared<GL::FragmentShader>(load_resource("shaders/bullet.f.glsl")))),
    beam_prog(new GL::Program(
      make_shared<GL::VertexShader>(load_resource("shaders/beam.v.glsl")),
      make_shared<GL::FragmentShader>(load_resource("shaders/beam.f.glsl")))),
    text_prog(new GL::Program(
      make_shared<GL::VertexShader>(load_resource("shaders/text.v.glsl")),
      make_shared<GL::FragmentShader>(load_resource("shaders/text.f.glsl"))))
{
	batches = {
		new TailBatch(*this),
		new ShipBatch(*this),
	};
	load_font();
}

void Renderer::load_font() {
	font_tex.bind();
	GL::check();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GL::check();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GL::check();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GL::check();
	const int n = 256;
	unsigned char data[64*n];
	for (int i = 0; i < n; i++) {
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				uint8 row = oort_font[8*i+y];
				bool on = ((row >> x) & 1) == 1;
				data[n*8*y + 8*i + x] = on ? 255 : 0;
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, n*8, 8, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	GL::check();
	GL::Texture::unbind();
	GL::check();
}

void Renderer::reshape(int screen_width, int screen_height) {
	this->screen_width = screen_width;
	this->screen_height = screen_height;
	this->aspect_ratio = float(screen_width)/screen_height;
}

void Renderer::render(float view_radius,
                      glm::vec2 view_center) {
	GL::check();

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.03f, 0.0f);
	glLineWidth(1.2f);

	glClear(GL_COLOR_BUFFER_BIT);

	p_matrix = glm::ortho(view_center.x - view_radius,
	                      view_center.x + view_radius,
	                      view_center.y - view_radius/aspect_ratio,
	                      view_center.y + view_radius/aspect_ratio);

	BOOST_FOREACH(auto batch, batches) {
		batch->render();
	}
	render_bullets();
	render_beams();
}

void Renderer::tick() {
	BOOST_FOREACH(auto batch, batches) {
		batch->tick();
	}
}

void Renderer::render_bullets() {
	auto &prog = *bullet_prog;
	boost::random::mt19937 prng(game->ticks);
	boost::random::normal_distribution<> p_dist(0.0, 0.5);

	vec4 colors[] = {
		vec4(0.27f, 0.27f, 0.27f, 0.33f),
		vec4(0.27f, 0.27f, 0.27f, 1.0f)
	};

	prog.use();
	GL::check();

	prog.enable_attrib_array("vertex");
	prog.enable_attrib_array("color");
	prog.uniform("p_matrix", p_matrix);
	prog.uniform("mv_matrix", glm::mat4());
	prog.attrib_ptr("color", colors);

	BOOST_FOREACH(auto bullet, game->bullets) {
		if (bullet->dead) {
			continue;
		}

		auto dp = bullet->get_velocity() * (1.0f/40);
		auto p1 = bullet->get_position() - dp;
		auto p2 = bullet->get_position();

		vec2 vertices[] = { p1, p2 };
		prog.attrib_ptr("vertex", vertices);
		glDrawArrays(GL_LINES, 0, 2);
	}

	prog.disable_attrib_array("vertex");
	prog.disable_attrib_array("color");
	GL::Program::clear();
	GL::check();
}

void Renderer::render_beams() {
	auto &prog = *beam_prog;
	prog.use();
	prog.uniform("p_matrix", p_matrix);
	prog.enable_attrib_array("vertex");
	prog.enable_attrib_array("texcoord");

	vec2 texcoords[] = {
		vec2(0, 1),
		vec2(0, 0),
		vec2(1, 1),
		vec2(1, 0)
	};

	prog.attrib_ptr("texcoord", texcoords);

	BOOST_FOREACH(auto beam, game->beams) {
		glm::vec4 color = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);

		glm::mat4 mv_matrix;
		auto p = beam->get_position();
		auto h = beam->get_heading();
		mv_matrix = glm::translate(mv_matrix, glm::vec3(p.x, p.y, 0));
		mv_matrix = glm::rotate(mv_matrix, glm::degrees(h), glm::vec3(0, 0, 1));

		auto &def = beam->get_def();
		vec2 vertices[] = {
			vec2(0, def.width/2.0f),
			vec2(0, -def.width/2.0f),
			vec2(def.length, def.width/2.0f),
			vec2(def.length, -def.width/2.0f)
		};

		prog.uniform("mv_matrix", mv_matrix);
		prog.uniform("color", color);

		prog.attrib_ptr("vertex", vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	prog.disable_attrib_array("vertex");
	prog.disable_attrib_array("texcoord");
	GL::Program::clear();
}

// XXX
static constexpr float screen_width = 1600;
static constexpr float screen_height = 900;

static vec2 pixel2screen(vec2 p) {
	return vec2((float) (2*p.x/screen_width-1),
	            (float) (-2*p.y/screen_height+1));
}

void Renderer::text(int x, int y, const std::string &str) {
	auto pos = pixel2screen(vec2(x,y));
	auto spacing = 9.0f;
	auto n = str.length();

	std::vector<float> data(2*n);
	for (unsigned int i = 0; i < n; i++) {
		data[2*i] = float(str[i]); // character
		data[2*i+1] = float(i); // index
	}

	text_prog->use();
	font_tex.bind();
	text_prog->uniform("tex", 0);
	text_prog->uniform("dist", 2.0f*spacing/screen_width);
	text_prog->uniform("position", pos);
	text_prog->attrib_ptr("character", &data[0], 8);
	text_prog->attrib_ptr("index", &data[1], 8);
	text_prog->enable_attrib_array("character");
	text_prog->enable_attrib_array("index");
	glDrawArrays(GL_POINTS, 0, n);
	text_prog->disable_attrib_array("character");
	text_prog->disable_attrib_array("index");
	GL::Texture::unbind();
	GL::Program::clear();
	GL::check();
}

}
