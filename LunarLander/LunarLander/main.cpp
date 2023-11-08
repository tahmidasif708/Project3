/**
* Author: Tahmid Asif
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>
#include "Entity.hpp"

#define PLATFORM_COUNT 13
#define OBS_COUNT 3

struct GameState {
    Entity *player;
    Entity *platforms;
};

GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


GLuint load_texture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    // light cyan
    glClearColor(1.9f, 1.0f, 1.0f, 1.0f); // Change color
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
    // Initialize Game Objects
    
    // Initialize Bird
    g_game_state.player = new Entity();
    g_game_state.player->position = glm::vec3(0, 4.5f, 0);
    g_game_state.player->movement = glm::vec3(0);
    g_game_state.player->acceleration = glm::vec3(0, -0.91f, 0);
    g_game_state.player->speed = 1.5f;
    g_game_state.player->textureID = load_texture("bird_lander.png");
        
    // Initialize Base Tiles
    g_game_state.platforms = new Entity[PLATFORM_COUNT + OBS_COUNT];
    GLuint basePlatformTextureID = load_texture("platformPack_tile002.png");
    GLuint grassPlatformTextureID = load_texture("platformPack_tile045.png");
    GLuint obstaclePlatformTextureID = load_texture("platformPack_tile039.png");
    
    float x_pos_base = -4.5f;
    float x_pos = -2.5f;
    float y_pos = 1.5f;
    
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        // platforms 6, 7, 8
        if (i > 5 && i < 9) {
            g_game_state.platforms[i].textureID = grassPlatformTextureID;
            g_game_state.platforms[i].position = glm::vec3(x_pos_base, -3.25, 0);
            g_game_state.platforms[i].height = 0.35;
            x_pos_base += 1;
        }
        else if (i > 9) {
            g_game_state.platforms[i].textureID = obstaclePlatformTextureID;
            g_game_state.platforms[i].position = glm::vec3(x_pos, y_pos, 0);
            x_pos += 2.5;
            y_pos -= 1;
        }
        else {
            g_game_state.platforms[i].textureID = basePlatformTextureID;
            g_game_state.platforms[i].position = glm::vec3(x_pos_base, -3.25, 0);
            x_pos_base += 1;
        }
    }
    
    for (int i = 0; i < (PLATFORM_COUNT + OBS_COUNT); i++ ){
        g_game_state.platforms[i].update(0, NULL, 0);
    }
}

void draw_text(ShaderProgram *g_shader_program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });
        
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });

    } // end of for loop
    
    glm::mat4 fontModelMatrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(fontModelMatrix, position);
    g_shader_program->set_model_matrix(fontModelMatrix);
    
    glUseProgram(g_shader_program->get_program_id());

    glVertexAttribPointer(g_shader_program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(g_shader_program->positionAttribute);

    glVertexAttribPointer(g_shader_program->get_tex_coordinate_attribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute);

    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(g_shader_program->positionAttribute);
    glDisableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute);
}

void process_input() {
    
    g_game_state.player->movement = glm::vec3(0);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        // Move the player left
                        break;
                        
                    case SDLK_RIGHT:
                        // Move the player right
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (g_game_state.player->collidedBottom == false) {
        if (keys[SDL_SCANCODE_LEFT]) {
            g_game_state.player->movement.x = -1.0f;
            g_game_state.player->acceleration.y = -0.5f;
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            g_game_state.player->movement.x = 1.0f;
            g_game_state.player->acceleration.y = -0.5f;
        }
        
        if (glm::length(g_game_state.player->movement) > 1.0f) {
            g_game_state.player->movement = glm::normalize(g_game_state.player->movement);
        }
    }
    else if ((g_game_state.player->CheckCollision(&g_game_state.platforms[6])) || (g_game_state.player->CheckCollision(&g_game_state.platforms[7])) || (g_game_state.player->CheckCollision(&g_game_state.platforms[8]))) {
        
        g_game_state.player->collidedGrass = true;
        std::cout << "Bird is Safe!!\n";
    }
    else {
        g_game_state.player->isSafe = false;
        std::cout << "Game Over\n";
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platforms, PLATFORM_COUNT);
        deltaTime -= FIXED_TIMESTEP;
    }
    
    accumulator = deltaTime;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        g_game_state.platforms[i].render(&g_shader_program);
    }
    
    if (g_game_state.player->collidedGrass == true) {
        draw_text(&g_shader_program, load_texture("font1.png"), "Mission Successful!", 0.5f, -0.25f, glm::vec3(0, 0, 0));
        g_game_state.player->isActive = false;
    }
    else if (g_game_state.player->isSafe == false) {
        draw_text(&g_shader_program, load_texture("font1.png"), "Mission Failed :-(", 0.5f, -0.25f, glm::vec3(0, 0, 0));
        g_game_state.player->isActive = false;
    }
    
    g_game_state.player->render(&g_shader_program);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialise();
    
    while (g_game_is_running) {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
