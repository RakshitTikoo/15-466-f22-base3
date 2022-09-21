#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint defender_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > defender_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("Defender.pnct"));
	defender_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > defender_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("Defender.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = defender_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = defender_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > Main_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Main.opus"));
});

Load< Sound::Sample > Fast_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Fast.opus"));
});

// Init State
void PlayMode::init(int state) {
	if(state == 0) {
		// =======================
		// Set initial positions
		// =======================

		camera->transform->position.x = -3.433114f;
		camera->transform->position.y = 19.637268f;
		camera->transform->position.z = 8.183211f;

		camera->transform->rotation.w = 0.013100f;
		camera->transform->rotation.x = 0.002377f;
		camera->transform->rotation.y = -0.653258f;
		camera->transform->rotation.z = -0.757018f;

		Monster_Life = 100;
		Robot_Life = 100;
		Init_State_Msg = "Press Space to Begin";
		
	}
	else {
		// =====================
		// Game Over Message
		// =====================

		if(Monster_Life <= 0 && Robot_Life > 0) Init_State_Msg = "P1 : Win || P2 : Loss";
		if(Robot_Life <= 0 && Monster_Life > 0) Init_State_Msg = "P1 : Loss || P2 : Win";
		if(Monster_Life <= 0 && Robot_Life <= 0) Init_State_Msg = "P1 : Draw || P2 : Draw";
	}


	// Set all control values

	Robot_Correct = 0; Monster_Correct = 0; 
	wobble = 0.0f;
	shift_val = 0.0f;

	main_volume = 1.0f;
	fast_volume = 0.0f;

	
	update_counter = 0.0f;
	update_time = 1.0f/(1.0f*2.5f);

	music_switch_flg = 0;
	fast_mode = 0; //If 0, select 1


	Robot_keys = "";
	Robot_arm_state = 0;
	Robot_randnum = 5;


	Monster_keys = "";
	Monster_arm_state = 0;
	Monster_randnum = 5;

	init_flag = 1;
	
	Monster_Life_String = "";
	Robot_Life_String = "";


	is_init = 1;
	
	main_volume = 0.0f;
	fast_volume = 0.0f;

	choice_taken = 0;
	Monster_hit_rate = 2;
	Robot_hit_rate = 2;

	hit_flg = 0;

	Monster_Power = 50;
	Robot_Power = 50;

	Robot_Power_en = 0;
	Monster_Power_en = 0;


	Robot_Power_Available = "P1 Power Available";
	Monster_Power_Available = "P2 Power Available";
	
	
}



// ================
// Get transforms
// ================

void PlayMode::get_transforms() {
	for (auto &transform : scene.transforms) {
		// Environment
		if (transform.name == "Platform") platform = &transform;
		else if (transform.name == "Buildings") buildings = &transform;
		else if (transform.name == "Windows") windows = &transform;

		// Robot
		else if (transform.name == "Robot_Arm_L") robot.left_arm = &transform;
		else if (transform.name == "Robot_Arm_R") robot.right_arm = &transform;
		else if (transform.name == "Robot_Forearm_L") robot.left_forearm = &transform;
		else if (transform.name == "Robot_Forearm_R") robot.right_forearm = &transform;
		else if (transform.name == "Robot_Head") robot.head = &transform;
		else if (transform.name == "Robot_Leg_L") robot.left_leg = &transform;
		else if (transform.name == "Robot_Leg_R") robot.right_leg = &transform;
		else if (transform.name == "Robot_Torso") robot.torso = &transform;

		// Monster
		else if (transform.name == "Monster_Arm_L") monster.left_arm = &transform;
		else if (transform.name == "Monster_Arm_R") monster.right_arm = &transform;
		else if (transform.name == "Monster_Forearm_L") monster.left_forearm = &transform;
		else if (transform.name == "Monster_Forearm_R") monster.right_forearm = &transform;
		else if (transform.name == "Monster_Head") monster.head = &transform;
		else if (transform.name == "Monster_Leg_L") monster.left_leg = &transform;
		else if (transform.name == "Monster_Leg_R") monster.right_leg = &transform;
		else if (transform.name == "Monster_Torso") monster.torso = &transform;

	}
	
	if (platform == nullptr) throw std::runtime_error("platform not found.");
	if (buildings == nullptr) throw std::runtime_error("buildings not found.");
	if (windows == nullptr) throw std::runtime_error("windows not found.");

	if (robot.left_arm == nullptr) throw std::runtime_error("robot.left_arm not found.");
	if (robot.right_arm == nullptr) throw std::runtime_error("robot.right_arm not found.");
	if (robot.left_forearm == nullptr) throw std::runtime_error("robot.left_forearm not found.");
	if (robot.right_forearm == nullptr) throw std::runtime_error("robot.right_forearm not found.");
	if (robot.head == nullptr) throw std::runtime_error("robot.head not found.");
	if (robot.left_leg == nullptr) throw std::runtime_error("robot.left_leg not found.");
	if (robot.right_leg == nullptr) throw std::runtime_error("robot.right_leg not found.");
	if (robot.torso == nullptr) throw std::runtime_error("robot.torso not found.");

	if (monster.left_arm == nullptr) throw std::runtime_error("monster.left_arm not found.");
	if (monster.right_arm == nullptr) throw std::runtime_error("monster.right_arm not found.");
	if (monster.left_forearm == nullptr) throw std::runtime_error("monster.left_forearm not found.");
	if (monster.right_forearm == nullptr) throw std::runtime_error("monster.right_forearm not found.");
	if (monster.head == nullptr) throw std::runtime_error("monster.head not found.");
	if (monster.left_leg == nullptr) throw std::runtime_error("monster.left_leg not found.");
	if (monster.right_leg == nullptr) throw std::runtime_error("monster.right_leg not found.");
	if (monster.torso == nullptr) throw std::runtime_error("monster.torso not found.");


	// Get initial rotation values
	robot_left_arm_rotation = robot.left_arm->rotation;
	robot_left_forearm_rotation = robot.left_forearm->rotation;
	robot_left_arm_position = robot.left_arm->position;
	robot_left_forearm_position = robot.left_forearm->position;

	robot_right_arm_rotation = robot.right_arm->rotation;
	robot_right_forearm_rotation = robot.right_forearm->rotation;
	robot_right_arm_position = robot.right_arm->position;
	robot_right_forearm_position = robot.right_forearm->position;

	monster_left_arm_rotation = monster.left_arm->rotation;
	monster_left_forearm_rotation = monster.left_forearm->rotation;
	monster_left_arm_position = monster.left_arm->position;
	monster_left_forearm_position = monster.left_forearm->position;

	monster_right_arm_rotation = monster.right_arm->rotation;
	monster_right_forearm_rotation = monster.right_forearm->rotation;
	monster_right_arm_position = monster.right_arm->position;
	monster_right_forearm_position = monster.right_forearm->position;

		
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}


// =====================================
// Function to control player movements
// =====================================

void PlayMode::punch_animation(float wobble_val, float shift, int Robot_State, int Monster_State) {

	// Robot States - 0,1,2
	if(Robot_State == 0) {
		robot.left_arm->rotation = robot_left_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		robot.left_forearm->rotation = robot_left_forearm_rotation * glm::angleAxis(
		glm::radians(-20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		robot.left_arm->position.x = robot_left_arm_position.x + shift;
		robot.left_forearm->position.x = robot_left_forearm_position.x + shift;
	}
	if(Robot_State == 1) {
		robot.right_arm->rotation = robot_right_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		robot.right_forearm->rotation = robot_right_forearm_rotation * glm::angleAxis(
		glm::radians(-20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		robot.right_arm->position.x = robot_right_arm_position.x + shift;
		robot.right_forearm->position.x = robot_right_forearm_position.x + shift;
	}
	if(Robot_State == 2) {
		robot.right_arm->rotation = robot_right_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		robot.right_forearm->rotation = robot_right_forearm_rotation * glm::angleAxis(
		glm::radians(40.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));

		robot.left_arm->rotation = robot_left_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		robot.left_forearm->rotation = robot_left_forearm_rotation * glm::angleAxis(
		glm::radians(40.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));

		robot.left_arm->position.x = robot_left_arm_position.x - shift*0.5f;
		robot.left_forearm->position.x = robot_left_forearm_position.x - shift*1.35f;
		robot.right_arm->position.x = robot_right_arm_position.x - shift*0.5f;
		robot.right_forearm->position.x = robot_right_forearm_position.x - shift*1.35f;

		robot.left_arm->position.z = robot_left_arm_position.z - shift*0.5f;
		robot.right_arm->position.z = robot_right_arm_position.z - shift*0.5f;
		robot.left_forearm->position.z = robot_left_forearm_position.z - shift*0.5f - shift;
		robot.right_forearm->position.z = robot_right_forearm_position.z - shift*0.5f - shift;
	}

	// Monster States - 0,1,2
	if(Monster_State == 0) {
		monster.left_arm->rotation = monster_left_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		monster.left_forearm->rotation = monster_left_forearm_rotation * glm::angleAxis(
		glm::radians(-20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		monster.left_arm->position.x = monster_left_arm_position.x - shift;
		monster.left_forearm->position.x = monster_left_forearm_position.x - shift;
	}
	if(Monster_State == 1) {
		monster.right_arm->rotation = monster_right_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		monster.right_forearm->rotation = monster_right_forearm_rotation * glm::angleAxis(
		glm::radians(-20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		monster.right_arm->position.x = monster_right_arm_position.x - shift;
		monster.right_forearm->position.x = monster_right_forearm_position.x - shift;
	}
	if(Monster_State == 2) {
		monster.right_arm->rotation = monster_right_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		monster.right_forearm->rotation = monster_right_forearm_rotation * glm::angleAxis(
		glm::radians(40.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));

		monster.left_arm->rotation = monster_left_arm_rotation * glm::angleAxis(
		glm::radians(20.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
		monster.left_forearm->rotation = monster_left_forearm_rotation * glm::angleAxis(
		glm::radians(40.0f * std::sin(wobble_val * 16.0f * 3.14159f)),
		glm::vec3(0.0f, 1.0f, 0.0f));

		monster.left_arm->position.x = monster_left_arm_position.x + shift*0.5f;
		monster.left_forearm->position.x = monster_left_forearm_position.x + shift*1.35f;
		monster.right_arm->position.x = monster_right_arm_position.x + shift*0.5f;
		monster.right_forearm->position.x = monster_right_forearm_position.x + shift*1.35f;

		monster.left_arm->position.z = monster_left_arm_position.z - shift*0.5f;
		monster.right_arm->position.z = monster_right_arm_position.z - shift*0.5f;
		monster.left_forearm->position.z = monster_left_forearm_position.z - shift*0.5f - shift;
		monster.right_forearm->position.z = monster_right_forearm_position.z - shift*0.5f - shift;
	}
	
}

PlayMode::PlayMode() : scene(*defender_scene) {
	get_transforms();
	init(0);

	//start music loop playing:
	// (note: position will be over-ridden in update())
	MainLoop = Sound::loop(*Main_sample, main_volume, 0.0f);
	FastLoop = Sound::loop(*Fast_sample, fast_volume, 0.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_a && a.debounce == 0) {
			a.downs += 1;
			a.pressed = true;
			a.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d && d.debounce == 0) {
			d.downs += 1;
			d.pressed = true;
			d.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w && w.debounce == 0) {
			w.downs += 1;
			w.pressed = true;
			w.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s && s.debounce == 0) {
			s.downs += 1;
			s.pressed = true;
			s.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT && left.debounce == 0) {
			left.downs += 1;
			left.pressed = true;
			left.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT && right.debounce == 0) {
			right.downs += 1;
			right.pressed = true;
			right.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP && up.debounce == 0) {
			up.downs += 1;
			up.pressed = true;
			up.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN && down.debounce == 0) {
			down.downs += 1;
			down.pressed = true;
			down.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE && space.debounce == 0) {
			space.downs += 1;
			space.pressed = true;
			space.debounce = 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SLASH && div.debounce == 0) {
			div.downs += 1;
			div.pressed = true;
			div.debounce = 1;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			a.pressed = false;
			a.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			d.pressed = false;
			d.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			w.pressed = false;
			w.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			s.pressed = false;
			s.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			left.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			right.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			up.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			down.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			space.debounce = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SLASH) {
			div.pressed = false;
			div.debounce = 0;
			return true;
		}
	} 

	return false;
}

void PlayMode::update(float elapsed) {
	if(!is_init) { // If not in init state
		if(!music_switch_flg) { // If not switching music currently

			// ==================
			// BPM update
			// ==================
			update_counter += elapsed;
			if(update_counter >= update_time) { // When hit the beat value
				update_counter -= update_time;

				// Robot update
				punch_animation(wobble, shift_val, Robot_arm_state, Monster_arm_state);
				wobble = 0;
				shift_val = 0.0f;


				// New keys to press are generated below
				if(Robot_keys == ""){
					// Reset flags 
					Robot_Correct = 0; Monster_Correct = 0; hit_flg = 0; choice_taken = 0;
					Robot_randnum = std::rand() % 4;
					if(Robot_array[Robot_randnum] == 0) Robot_keys = "W";
					if(Robot_array[Robot_randnum] == 1) Robot_keys = "A";
					if(Robot_array[Robot_randnum] == 2) Robot_keys = "S";
					if(Robot_array[Robot_randnum] == 3) Robot_keys = "D";

				}
				else 
					Robot_keys = "";


				if(Monster_keys == ""){
					// Reset flags 
					Robot_Correct = 0; Monster_Correct = 0; hit_flg = 0; choice_taken = 0;
					Monster_randnum = std::rand() % 4;
					if(Monster_array[Monster_randnum] == 0) Monster_keys = "Left";
					if(Monster_array[Monster_randnum] == 1) Monster_keys = "Right";
					if(Monster_array[Monster_randnum] == 2) Monster_keys = "Up";
					if(Monster_array[Monster_randnum] == 3) Monster_keys = "Down";

				}
				else 
					Monster_keys = "";


				// ====================
				// Update Power Value
				// ====================
				if(!Monster_Power_en && Monster_Power < 50) {
					Monster_Power += 1;
					if(Monster_Power == 50) Monster_Power_Available = "P2 Power Available";
				}
				if(!Robot_Power_en && Robot_Power < 50) {
					Robot_Power += 1;
					if(Robot_Power == 50) Robot_Power_Available = "P1 Power Available";
				}


				if(Monster_Power_en) {
					Monster_Power -= 1;
					if(Monster_Power <= 0) {
						music_switch_flg = 1;
						Monster_Power_en = 0; Robot_Power_en = 0; fast_mode = 0; 
					}

				}
				else if(Robot_Power_en) {
					Robot_Power -= 1;
					if(Robot_Power <= 0) {
						music_switch_flg = 1;
						Monster_Power_en = 0; Robot_Power_en = 0; fast_mode = 0; 
					}
				}

			}
			else if (choice_taken == 0){ // If we have not hit a key in current beat

				// =================
				// Key Press Check 
				// =================

				// WASD Press Check 
				if(w.pressed && w.debounce == 1) {
					choice_taken = 1;
					w.debounce = 2;
					if(Robot_array[Robot_randnum] == 0) Robot_Correct = 1;
					else Robot_Correct = 0;
				}
				else if(a.pressed && a.debounce == 1) {
					choice_taken = 1;
					a.debounce = 2;
					if(Robot_array[Robot_randnum] == 1) Robot_Correct = 1;
					else Robot_Correct = 0;
				}
				else if(s.pressed && s.debounce == 1) {
					choice_taken = 1;
					s.debounce = 2;
					if(Robot_array[Robot_randnum] == 2) Robot_Correct = 1;
					else Robot_Correct = 0;
				}
				else if(d.pressed && d.debounce == 1) {
					choice_taken = 1;
					d.debounce = 2;
					if(Robot_array[Robot_randnum] == 3) Robot_Correct = 1;
					else Robot_Correct = 0;
				}

				// Left Right Up Down Check
				if(left.pressed && left.debounce == 1) {
					choice_taken = 1;
					left.debounce = 2;
					if(Monster_array[Monster_randnum] == 0) Monster_Correct = 1;
					else Monster_Correct = 0;
				}
				else if(right.pressed && right.debounce == 1) {
					choice_taken = 1;
					right.debounce = 2;
					if(Monster_array[Monster_randnum] == 1) Monster_Correct = 1;
					else Monster_Correct = 0;
				}
				else if(up.pressed && up.debounce == 1) {
					choice_taken = 1;
					up.debounce = 2;
					if(Monster_array[Monster_randnum] == 2) Monster_Correct = 1;
					else Monster_Correct = 0;
				}
				else if(down.pressed && down.debounce == 1) {
					choice_taken = 1;
					down.debounce = 2;
					if(Monster_array[Monster_randnum] == 3) Monster_Correct = 1;
					else Monster_Correct = 0;
				}


			}


			// ===================
			// Fast Mode Enable
			// ===================
			if(space.pressed && space.debounce == 1 && Robot_Power == 50 && fast_mode == 0) {
				space.debounce = 2;
				music_switch_flg = 1;
				fast_mode = 1;
				Robot_Power_en = 1;
				Robot_Power_Available = "";
				Robot_hit_rate = 4;
			}
			else if(div.pressed && div.debounce == 1 && Monster_Power == 50 && fast_mode == 0) {
				div.debounce = 2;
				music_switch_flg = 1;
				fast_mode = 1;
				Monster_Power_en = 1;
				Monster_Power_Available = "";
				Monster_hit_rate = 4;
			}

		}
		else { // When music_switch_flg = 1
			
			// ================
			// Song Fade Out
			// ================

			wobble = 0.0f; shift_val = 0.0f;
			if(fast_mode == 1) {
				Monster_keys = "Fast Mode Engaged";
				Robot_keys = "Fast Mode Engaged";
				if(main_volume > 0.0f)	main_volume -= 0.02f;
				else if (fast_volume < 1.0f) fast_volume += 0.02f;

				if(main_volume < 0.0f) main_volume = 0.0f;
				if(fast_volume > 1.0f) fast_volume = 1.0f;

				update_time = 1.0f/(1.0f*3.0f); update_counter = 0.0f;
			}
			else {
				Monster_keys = "Switching to Normal Mode";
				Robot_keys = "Switching to Normal Mode";
				if(fast_volume > 0.0f)	fast_volume -= 0.02f;
				else if (main_volume < 1.0f) main_volume += 0.02f;

				if(fast_volume < 0.0f) fast_volume = 0.0f;
				if(main_volume > 1.0f) main_volume = 1.0f;

				update_time = 1.0f/(1.0f*2.5f); update_counter = 0.0f;
				Monster_hit_rate = 2;
				Robot_hit_rate = 2;
			}

			if((main_volume == 0.0f && fast_volume == 1.0f) || (main_volume == 1.0f && fast_volume == 0.0f)) {
				music_switch_flg = 0;
			}

		}

		// ================
		// Check hit cases
		// ================

		if (Robot_Correct == 1 && Monster_Correct == 0 && hit_flg == 0) {
			Monster_Life -= Robot_hit_rate;
			
			if(Robot_arm_state == 0) Robot_arm_state = 1;
			else if (Robot_arm_state == 1) Robot_arm_state = 0;
			else Robot_arm_state = 0;

			Monster_arm_state = 2;

			wobble = 0.04f;
			shift_val = -0.5f;
			hit_flg = 1;
		}
		if (Robot_Correct == 0 && Monster_Correct == 1 && hit_flg == 0) {
			Robot_Life -= Monster_hit_rate;
			
			if(Monster_arm_state == 0) Monster_arm_state = 1;
			else if (Monster_arm_state == 1) Monster_arm_state = 0;
			else Monster_arm_state = 0;

			Robot_arm_state = 2;

			wobble = 0.04f;
			shift_val = -0.5f;
			hit_flg = 1;
		}
		if(Robot_Correct == 1 && Monster_Correct == 1 && hit_flg == 0) {
			Monster_arm_state = 2;
			Robot_arm_state = 2;
			wobble = 0.04f;
			shift_val = -0.5f;
			hit_flg = 1;
		}


		// =================
		// Game Over Logic
		// =================

		if(Robot_Life <= 0 || Monster_Life <= 0) {init(1);}

	}
	else { 
		// =================
		// Begin Game Logic
		// =================

		if(space.pressed && space.debounce == 1) {
			space.debounce = 2;
			main_volume = 1.0f;
			fast_volume = 0.0f;
			is_init = 0;
			Init_State_Msg = "";

			Monster_Life = 100;
			Robot_Life = 100;
			}
		
	}
	

	// =================
	// Volume Setting
	// =================

	MainLoop->volume = main_volume;
	FastLoop->volume = fast_volume;
	
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	a.downs = 0;
	d.downs = 0;
	s.downs = 0;
	w.downs = 0;

	space.downs = 0;
	div.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));


		constexpr float H = 0.09f;
		float ofs = 850.0f / drawable_size.y;

		if(!is_init) {
			lines.draw_text(Robot_keys,
			glm::vec3(-aspect + 0.1f * H + 1.0f * ofs, -1.0 + 0.1f * H + 1.5f * ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		
			lines.draw_text(Monster_keys,
				glm::vec3(-aspect + 0.1f * H + 2.0f * ofs, -1.0 + + 0.1f * H + 1.5f * ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	
			
			Robot_Life_String = std::string("HP:") + std::to_string(Robot_Life); // To tell Monster HP		
			lines.draw_text(Robot_Life_String,
				glm::vec3(-aspect + 0.1f * H + 0.9f * ofs, -1.0 + 0.1f * H, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			
			Monster_Life_String = std::string("HP:") + std::to_string(Monster_Life); // To tell Monster HP		
			lines.draw_text(Monster_Life_String,
			glm::vec3(-aspect + 0.1f * H + 1.95f * ofs, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

			lines.draw_text(Robot_Power_Available, // To tell Robot Power Available
				glm::vec3(-aspect + 0.1f * H + 0.1f * ofs, -1.0 + 0.1f * H, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			
			lines.draw_text(Monster_Power_Available, // To tell Monster Power Available
			glm::vec3(-aspect + 0.1f * H + 2.4f * ofs, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));


		}
		else {
			lines.draw_text(Init_State_Msg, // To display init or game over message
			glm::vec3(-aspect + 0.1f * H + 1.2f * ofs, -1.0 + 12.0f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0xff, 0xff, 0x00));
		}
		
		

	
	}
	GL_ERRORS();
}
