#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	// Helper Functions
	void get_transforms();
	void init(int state);
	std::string Init_State_Msg;

	uint8_t is_init;
	uint8_t choice_taken;


	void punch_animation(float wobble_val, float shift, int Robot_State, int Monster_State); // 0 - right | 1 - left
	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
		uint8_t debounce = 0;
	} w, a, s, d, div, space, left, right, down, up;

	// To check which key was hit
	uint8_t Robot_Correct, Monster_Correct; 

	// Attack potency 
	uint32_t Monster_hit_rate, Robot_hit_rate;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	// Transforms for all objects
	Scene::Transform *platform;
	Scene::Transform *buildings;
	Scene::Transform *windows;
	Scene::Transform *reference;

	struct Player {
		Scene::Transform *left_leg;
		Scene::Transform *right_leg;
		Scene::Transform *head;
		Scene::Transform *torso;
		Scene::Transform *left_forearm;
		Scene::Transform *right_forearm;
		Scene::Transform *left_arm;
		Scene::Transform *right_arm;
	} robot, monster;


	// Movement Values
	float wobble;
	float shift_val;

	glm::quat robot_left_arm_rotation;
	glm::quat robot_left_forearm_rotation;	
	glm::vec3 robot_left_arm_position;
	glm::vec3 robot_left_forearm_position;
	glm::quat robot_right_arm_rotation;
	glm::quat robot_right_forearm_rotation;	
	glm::vec3 robot_right_arm_position;
	glm::vec3 robot_right_forearm_position;

	glm::quat monster_left_arm_rotation;
	glm::quat monster_left_forearm_rotation;	
	glm::vec3 monster_left_arm_position;
	glm::vec3 monster_left_forearm_position;
	glm::quat monster_right_arm_rotation;
	glm::quat monster_right_forearm_rotation;	
	glm::vec3 monster_right_arm_position;
	glm::vec3 monster_right_forearm_position;


	//music coming from the tip of the leg (as a demonstration):

	// Music Control Values
	std::shared_ptr< Sound::PlayingSample > MainLoop;
	float main_volume;
	std::shared_ptr< Sound::PlayingSample > FastLoop;
	float fast_volume;

	// Update Counter to meet BPM
	float update_counter;
	float update_time;

	uint8_t music_switch_flg; 
	uint8_t fast_mode; //If 0 - normal spped (150 bpm), else fast speed (180 bpm)


	// Power Control Values
	uint8_t Robot_Power_en, Monster_Power_en;
	std::string Robot_Power_Available, Monster_Power_Available;
	uint32_t Robot_Power; 
	uint32_t Monster_Power;

	// Hit Control Valueas
	std::string Robot_keys;
	uint8_t Robot_array[4] = {0,1,2,3}; // 0 - W | 1 - A | 2 - S | 3 - D
	int Robot_arm_state;
	uint8_t Robot_randnum;

	std::string Monster_keys;
	uint8_t Monster_array[4] = {0,1,2,3}; // 0 - left | 1 - right | 2 - up  | 3 - down
	int Monster_arm_state;
	uint8_t Monster_randnum;

	uint8_t hit_flg;

	// Game Logic Values
	uint8_t init_flag;
	int Monster_Life;
	int Robot_Life;
	std::string Monster_Life_String;
	std::string Robot_Life_String;

	//camera:
	Scene::Camera *camera = nullptr;

};
