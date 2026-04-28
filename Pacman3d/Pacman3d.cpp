#include "Pacman3d.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Ghost.h"
#include "Map.h"
#include "Player.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace
{
    constexpr int kWindowWidth = 800;
    constexpr int kWindowHeight = 600;

    bool readTextFile(const std::string& path, std::string& out)
    {
        std::ifstream file(path);
        if (!file)
        {
            file = std::ifstream("Pacman3d/" + path);
        }
        if (!file)
        {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        out = buffer.str();
        return true;
    }

    GLuint compileShader(GLenum type, const std::string& source)
    {
        const GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char log[512] = {};
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Shader compile error: " << log << "\n";
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::string vertexSource;
        std::string fragmentSource;
        if (!readTextFile(vertexPath, vertexSource) || !readTextFile(fragmentPath, fragmentSource))
        {
            std::cerr << "Failed to read shader files from ./shaders\n";
            return 0;
        }

        const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char log[512] = {};
            glGetProgramInfoLog(program, 512, nullptr, log);
            std::cerr << "Shader link error: " << log << "\n";
            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }

    std::vector<Ghost> createGhosts()
    {
        std::vector<Ghost> ghosts;
        ghosts.emplace_back(GhostType::BLINKY, glm::ivec2(8, 1), glm::ivec2(8, 1), glm::vec3(1.0f, 0.1f, 0.1f));
        ghosts.emplace_back(GhostType::BLINKY, glm::ivec2(1, 1), glm::ivec2(1, 1), glm::vec3(1.0f, 0.4f, 0.7f));
        ghosts.emplace_back(GhostType::BLINKY, glm::ivec2(8, 7), glm::ivec2(8, 7), glm::vec3(0.2f, 1.0f, 1.0f));
        ghosts.emplace_back(GhostType::BLINKY, glm::ivec2(1, 7), glm::ivec2(1, 7), glm::vec3(1.0f, 0.5f, 0.1f));
        return ghosts;
    }
}

int main()
{
    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Pacman 3D", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    const GLuint shaderProgram = createShaderProgram("shaders/basic.vert", "shaders/basic.frag");
    const GLuint skinnedShaderProgram = createShaderProgram("shaders/skinned.vert", "shaders/skinned.frag");
    if (shaderProgram == 0 || skinnedShaderProgram == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    Map map;
    Player player;
    std::vector<Ghost> ghosts = createGhosts();

    const auto& grid = map.getGrid();
    const float mapCenterX = grid.empty() ? 0.0f : static_cast<float>(grid[0].size() - 1) * 0.5f;
    const float mapCenterZ = grid.empty() ? 0.0f : static_cast<float>(grid.size() - 1) * 0.5f;
    const glm::vec3 cameraTarget(mapCenterX, 0.0f, mapCenterZ);
    const glm::vec3 cameraPos(mapCenterX, 11.5f, mapCenterZ + 8.0f);
    const glm::mat4 projection = glm::perspective(
        glm::radians(50.0f), static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight), 0.1f, 100.0f);

    float lastTime = static_cast<float>(glfwGetTime());
    float itemSpawnTimer = 0.0f;
    int score = 0;
    int level = 1;
    bool gameOver = false;
    bool gameWon = false;
    bool gameOverPopupOpened = false;
    bool gameWonPopupOpened = false;

    while (!glfwWindowShouldClose(window))
    {
        const float now = static_cast<float>(glfwGetTime());
        const float deltaTime = now - lastTime;
        lastTime = now;

        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!gameOver && !gameWon)
        {
            player.update(window, deltaTime, map);
            player.updateAnimation(deltaTime);

            const glm::ivec2 playerCell = player.getGridPosition();
            const int consumedItem = map.consumeItemAt(playerCell.x, playerCell.y);
            if (consumedItem == 2)
            {
                score += 10;
            }
            else if (consumedItem == 3)
            {
                score += 50;
                for (Ghost& ghost : ghosts)
                {
                    ghost.triggerFrightened(6.0f);
                }
            }
            else if (consumedItem == 4)
            {
                score += 250;
            }

            itemSpawnTimer += deltaTime;
            if (itemSpawnTimer >= 8.0f)
            {
                itemSpawnTimer = 0.0f;
                map.spawnRandomItem();
            }

            for (Ghost& ghost : ghosts)
            {
                ghost.update(deltaTime, map, player);
                ghost.updateAnimation(deltaTime);
                const GhostState stateBeforeCollision = ghost.getState();
                if (ghost.checkCollisionWithPlayer(player, map) && stateBeforeCollision == GhostState::FRIGHTENED)
                {
                    score += 200;
                }
            }

            gameOver = player.getLives() <= 0;
            gameWon = !map.hasRemainingDots();
        }

        ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.45f);
        ImGui::Begin("HUD", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoMove);
        ImGui::Text("Score: %d", score);
        ImGui::Text("Lives: %d", player.getLives());
        ImGui::Text("Level: %d", level);
        ImGui::End();

        if (gameOver && !gameOverPopupOpened)
        {
            ImGui::OpenPopup("GAME OVER");
            gameOverPopupOpened = true;
        }
        if (gameWon && !gameWonPopupOpened)
        {
            ImGui::OpenPopup("YOU WIN!");
            gameWonPopupOpened = true;
        }

        if (ImGui::BeginPopupModal("GAME OVER", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("GAME OVER");
            ImGui::Separator();

            if (ImGui::Button("Restart", ImVec2(120.0f, 0.0f)))
            {
                map.reset();
                player.reset();
                ghosts = createGhosts();
                score = 0;
                level = 1;
                itemSpawnTimer = 0.0f;
                gameOver = false;
                gameWon = false;
                gameOverPopupOpened = false;
                gameWonPopupOpened = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Exit", ImVec2(120.0f, 0.0f)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("YOU WIN!", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("YOU WIN!");
            ImGui::Separator();

            if (ImGui::Button("Next Level", ImVec2(120.0f, 0.0f)))
            {
                ++level;
                map.reset();
                player.reset();
                ghosts = createGhosts();
                itemSpawnTimer = 0.0f;
                gameOver = false;
                gameWon = false;
                gameOverPopupOpened = false;
                gameWonPopupOpened = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Exit", ImVec2(120.0f, 0.0f)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::EndPopup();
        }

        std::stringstream title;
        title << "Pacman 3D - Score: " << score << " | Lives: " << player.getLives();
        if (gameWon)
        {
            title << " | YOU WIN";
        }
        else if (gameOver)
        {
            title << " | GAME OVER";
        }
        glfwSetWindowTitle(window, title.str().c_str());

        glClearColor(0.05f, 0.08f, 0.20f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        map.drawMap(shaderProgram);

        glUseProgram(skinnedShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(skinnedShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skinnedShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        player.draw(skinnedShaderProgram);
        for (const Ghost& ghost : ghosts)
        {
            ghost.draw(skinnedShaderProgram);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteProgram(shaderProgram);
    glDeleteProgram(skinnedShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
