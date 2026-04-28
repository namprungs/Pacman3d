#include "Ghost.h"

#include "Animation.h"
#include "Animator.h"
#include "Map.h"
#include "Model.h"
#include "Player.h"

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    struct GhostAssets
    {
        std::shared_ptr<Model> model;
        std::shared_ptr<Animation> runAnimation;
        std::shared_ptr<Animation> crawlAnimation;
        std::shared_ptr<Animation> attackAnimation;
        std::shared_ptr<Animation> neckBiteAnimation;
    };

    GhostAssets& getGhostAssets()
    {
        static GhostAssets assets;
        if (!assets.model)
        {
            assets.model = std::make_shared<Model>("resources/objects/zombie/zombie_run.dae");
            assets.runAnimation = std::make_shared<Animation>("resources/objects/zombie/zombie_run.dae", assets.model.get());
            assets.crawlAnimation = std::make_shared<Animation>("resources/objects/zombie/low_crawl.dae", assets.model.get());
            assets.attackAnimation = std::make_shared<Animation>("resources/objects/zombie/zombie_attack.dae", assets.model.get());
            assets.neckBiteAnimation = std::make_shared<Animation>("resources/objects/zombie/zombie_neck_bite.dae", assets.model.get());
        }
        return assets;
    }
}

Ghost::Ghost(GhostType type, const glm::ivec2& startGridPosition, const glm::ivec2& scatterCorner, const glm::vec3& color)
    : type_(type),
      state_(GhostState::SCATTER),
      currentGridPosition_(startGridPosition),
      targetGridPosition_(startGridPosition),
      previousGridPosition_(startGridPosition),
      scatterCorner_(scatterCorner),
      spawnGridPosition_(startGridPosition),
      position_(static_cast<float>(startGridPosition.x), 0.35f, static_cast<float>(startGridPosition.y)),
      halfExtents_(0.30f, 0.30f, 0.30f),
      color_(color),
      rotationYDegrees_(0.0f),
      moveSpeed_(1.2f),
      frightenedTimer_(0.0f),
      stateTimer_(0.0f)
{
    const GhostAssets& assets = getGhostAssets();
    model_ = assets.model;
    runAnimation_ = assets.runAnimation;
    crawlAnimation_ = assets.crawlAnimation;
    attackAnimation_ = assets.attackAnimation;
    neckBiteAnimation_ = assets.neckBiteAnimation;
    animator_ = std::make_unique<Animator>(runAnimation_.get());
}

Ghost::~Ghost() = default;

Ghost::Ghost(Ghost&& other) noexcept = default;

Ghost& Ghost::operator=(Ghost&& other) noexcept = default;

void Ghost::update(float deltaTime, const Map& map, const Player& player)
{
    if (frightenedTimer_ > 0.0f)
    {
        frightenedTimer_ -= deltaTime;
        if (frightenedTimer_ > 0.0f)
        {
            state_ = GhostState::FRIGHTENED;
            setFrightened(true);
        }
        else
        {
            frightenedTimer_ = 0.0f;
            state_ = GhostState::CHASE;
            stateTimer_ = 0.0f;
            setFrightened(false);
        }
    }
    else
    {
        stateTimer_ += deltaTime;
        if (state_ == GhostState::CHASE && stateTimer_ >= 7.0f)
        {
            state_ = GhostState::SCATTER;
            stateTimer_ = 0.0f;
        }
        else if (state_ == GhostState::SCATTER && stateTimer_ >= 5.0f)
        {
            state_ = GhostState::CHASE;
            stateTimer_ = 0.0f;
        }
        //state_ = GhostState::CHASE;
    }

    const glm::vec3 targetWorldPosition(static_cast<float>(targetGridPosition_.x), 0.35f, static_cast<float>(targetGridPosition_.y));
    glm::vec3 toTarget = targetWorldPosition - position_;
    const float distanceToTarget = glm::length(toTarget);

    const float speedMultiplier = (state_ == GhostState::FRIGHTENED) ? 0.7f : 1.0f;
    const float step = moveSpeed_ * speedMultiplier * deltaTime;

    if (distanceToTarget <= step || distanceToTarget < 0.001f)
    {
        position_ = targetWorldPosition;
        previousGridPosition_ = currentGridPosition_;
        currentGridPosition_ = targetGridPosition_;
        targetGridPosition_ = chooseNextCell(map, player);
    }
    else
    {
        toTarget = glm::normalize(toTarget);
        position_ += toTarget * step;
    }

    const glm::vec3 movementDirection = glm::vec3(static_cast<float>(targetGridPosition_.x), 0.35f, static_cast<float>(targetGridPosition_.y)) - position_;
    if (glm::length(movementDirection) > 0.0001f)
    {
        const glm::vec3 dir = glm::normalize(movementDirection);
        rotationYDegrees_ = glm::degrees(std::atan2(dir.x, dir.z));
    }

    if (attackTimer_ > 0.0f)
    {
        attackTimer_ -= deltaTime;
        if (attackTimer_ <= 0.0f)
        {
            updateAnimationState();
        }
    }
}

void Ghost::updateAnimation(float deltaTime)
{
    if (animator_)
    {
        animator_->UpdateAnimation(deltaTime * 0.7f);
    }
}

void Ghost::draw(GLuint shaderProgram) const
{
    const GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    const GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    glm::mat4 model(1.0f);
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(rotationYDegrees_), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(modelScale_));

    glm::vec3 drawColor = color_;
    if (state_ == GhostState::FRIGHTENED)
    {
        drawColor = glm::vec3(0.2f, 0.2f, 1.0f);
    }

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniform3f(colorLoc, drawColor.r, drawColor.g, drawColor.b);

    if (animator_)
    {
        const auto& transforms = animator_->GetFinalBoneMatrices();
        for (std::size_t i = 0; i < transforms.size(); ++i)
        {
            std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &transforms[i][0][0]);
        }
    }

    if (model_)
    {
        model_->Draw(shaderProgram);
    }
}

void Ghost::triggerFrightened(float durationSeconds)
{
    frightenedTimer_ = durationSeconds;
    state_ = GhostState::FRIGHTENED;
    attackTimer_ = 0.0f;
    setFrightened(true);
}

bool Ghost::checkCollisionWithPlayer(Player& player, const Map& map)
{
    const glm::vec3 playerHalfExtents(0.35f, 0.35f, 0.35f);
    const glm::vec3 playerMin = player.getPosition() - playerHalfExtents;
    const glm::vec3 playerMax = player.getPosition() + playerHalfExtents;

    const glm::vec3 ghostMin = position_ - halfExtents_;
    const glm::vec3 ghostMax = position_ + halfExtents_;

    if (!intersectsAabb(playerMin, playerMax, ghostMin, ghostMax))
    {
        return false;
    }

    if (state_ == GhostState::FRIGHTENED)
    {
        respawnAwayFromPlayer(map, player);
        return true;
    }

    player.loseLife();
    triggerAttack();
    return true;
}

void Ghost::reset()
{
    state_ = GhostState::SCATTER;
    currentGridPosition_ = spawnGridPosition_;
    targetGridPosition_ = spawnGridPosition_;
    previousGridPosition_ = spawnGridPosition_;
    position_ = glm::vec3(static_cast<float>(spawnGridPosition_.x), 0.35f, static_cast<float>(spawnGridPosition_.y));
    rotationYDegrees_ = 0.0f;
    frightenedTimer_ = 0.0f;
    stateTimer_ = 0.0f;
    attackTimer_ = 0.0f;
    setFrightened(false);
    updateAnimationState();
}

void Ghost::setFrightened(bool frightened)
{
    if (isFrightened_ == frightened)
    {
        return;
    }

    isFrightened_ = frightened;
    updateAnimationState();
}

void Ghost::triggerAttack()
{
    useNeckBite_ = !useNeckBite_;
    Animation* attack = useNeckBite_ ? neckBiteAnimation_.get() : attackAnimation_.get();
    if (animator_ && attack)
    {
        animator_->PlayAnimation(attack);
        const float duration = attack->GetDuration();
        const float ticks = attack->GetTicksPerSecond();
        attackDuration_ = (ticks > 0.0f) ? (duration / ticks) : 0.0f;
        attackTimer_ = attackDuration_ > 0.0f ? attackDuration_ : 0.8f;
    }
}

void Ghost::updateAnimationState()
{
    if (!animator_)
    {
        return;
    }

    if (attackTimer_ > 0.0f)
    {
        return;
    }

    if (isFrightened_)
    {
        animator_->PlayAnimation(crawlAnimation_.get());
    }
    else
    {
        animator_->PlayAnimation(runAnimation_.get());
    }
}

void Ghost::respawnAwayFromPlayer(const Map& map, const Player& player)
{
    std::vector<glm::ivec2> candidates;
    const auto& grid = map.getGrid();
    const glm::ivec2 playerCell = player.getGridPosition();

    for (std::size_t z = 0; z < grid.size(); ++z)
    {
        for (std::size_t x = 0; x < grid[z].size(); ++x)
        {
            if (grid[z][x] == 1)
            {
                continue;
            }

            const glm::ivec2 cell(static_cast<int>(x), static_cast<int>(z));
            if (glm::length(glm::vec2(cell - playerCell)) < 3.0f)
            {
                continue;
            }

            candidates.push_back(cell);
        }
    }

    glm::ivec2 respawnCell = spawnGridPosition_;
    if (!candidates.empty())
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
        respawnCell = candidates[dist(rng)];
    }

    currentGridPosition_ = respawnCell;
    targetGridPosition_ = respawnCell;
    previousGridPosition_ = respawnCell;
    position_ = glm::vec3(static_cast<float>(respawnCell.x), 0.35f, static_cast<float>(respawnCell.y));
    frightenedTimer_ = 0.0f;
    state_ = GhostState::SCATTER;
    stateTimer_ = 0.0f;
    setFrightened(false);
    updateAnimationState();
}

glm::vec3 Ghost::getPosition() const
{
    return position_;
}

GhostState Ghost::getState() const
{
    return state_;
}

glm::ivec2 Ghost::chooseNextCell(const Map& map, const Player& player) const
{
    static const glm::ivec2 directions[] = {
        glm::ivec2(0, -1),
        glm::ivec2(1, 0),
        glm::ivec2(0, 1),
        glm::ivec2(-1, 0)};

    std::vector<glm::ivec2> candidates;
    for (const glm::ivec2& dir : directions)
    {
        const glm::ivec2 next = currentGridPosition_ + dir;
        if (!isWalkableCell(next, map))
        {
            continue;
        }
        if (next == previousGridPosition_)
        {
            continue;
        }
        candidates.push_back(next);
    }

    if (candidates.empty())
    {
        for (const glm::ivec2& dir : directions)
        {
            const glm::ivec2 next = currentGridPosition_ + dir;
            if (isWalkableCell(next, map))
            {
                candidates.push_back(next);
            }
        }
    }

    if (candidates.empty())
    {
        return currentGridPosition_;
    }

    const glm::ivec2 playerCell = player.getGridPosition();
    glm::ivec2 targetCell = scatterCorner_;

    if (state_ == GhostState::CHASE)
    {
        targetCell = getChaseTarget(player);
    }
    else if (state_ == GhostState::FRIGHTENED)
    {
        float farthestDistance = -1.0f;
        glm::ivec2 best = candidates.front();
        for (const glm::ivec2& candidate : candidates)
        {
            const glm::vec2 delta = glm::vec2(candidate - playerCell);
            const float dist = glm::length(delta);
            if (dist > farthestDistance)
            {
                farthestDistance = dist;
                best = candidate;
            }
        }
        return best;
    }

    float bestDistance = std::numeric_limits<float>::max();
    glm::ivec2 best = candidates.front();
    for (const glm::ivec2& candidate : candidates)
    {
        const glm::vec2 delta = glm::vec2(candidate - targetCell);
        const float dist = glm::length(delta);
        if (dist < bestDistance)
        {
            bestDistance = dist;
            best = candidate;
        }
    }

    return best;
}

glm::ivec2 Ghost::getChaseTarget(const Player& player) const
{
    const glm::ivec2 playerCell = player.getGridPosition();
    const glm::vec3 forward = player.getForwardDirection();
    const glm::ivec2 lookAhead(static_cast<int>(std::lround(forward.x * 2.0f)), static_cast<int>(std::lround(forward.z * 2.0f)));

    switch (type_)
    {
    case GhostType::BLINKY:
        return playerCell;
    case GhostType::PINKY:
        return playerCell + lookAhead;
    case GhostType::INKY:
        return playerCell + lookAhead + glm::ivec2(2, -2);
    case GhostType::CLYDE:
    {
        const glm::vec2 delta = glm::vec2(currentGridPosition_ - playerCell);
        if (glm::length(delta) < 3.0f)
        {
            return scatterCorner_;
        }
        return playerCell;
    }
    }

    return playerCell;
}

bool Ghost::isWalkableCell(const glm::ivec2& gridCell, const Map& map) const
{
    const auto& grid = map.getGrid();
    if (gridCell.y < 0 || gridCell.y >= static_cast<int>(grid.size()))
    {
        return false;
    }
    if (gridCell.x < 0 || gridCell.x >= static_cast<int>(grid[gridCell.y].size()))
    {
        return false;
    }
    return grid[gridCell.y][gridCell.x] != 1;
}

bool Ghost::intersectsAabb(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB) const
{
    return minA.x <= maxB.x &&
           maxA.x >= minB.x &&
           minA.y <= maxB.y &&
           maxA.y >= minB.y &&
           minA.z <= maxB.z &&
           maxA.z >= minB.z;
}
