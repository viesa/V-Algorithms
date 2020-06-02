#pragma once

#include <optional>

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include "Window.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Clock.h"
#include "Lib.h"
#include "VectorLib.h"

class Camera
{
public:
    Camera();
    ~Camera();

    static void Update();
    static void Draw(const sf::Drawable &drawable, sf::RenderStates renderStates = sf::RenderStates::Default) noexcept;

    static void Move(const sf::Vector2f &offset) noexcept;
    static void Zoom(float factor) noexcept;
    static void Rotate(float angle) noexcept;

    static void SetCenter(const sf::Vector2f &center) noexcept;
    static void SetZoom(float factor) noexcept;
    static void SetRotation(float angle) noexcept;

    static void Follow(sf::Vector2f *follow) noexcept { m_follow = follow; }
    static void Unfollow() noexcept { m_follow.reset(); }

    ///Translate a point to world space
    ///@param point: point to be translated from screen to world space.
    static sf::Vector2f ScreenToWorld(const sf::Vector2f &point) noexcept;
    ///Translate a point to world space
    ///@param point: point to be translated from world to screen space.
    static sf::Vector2f WorldToScreen(const sf::Vector2f &point) noexcept;

    void SetRotationSpeed(float rps) noexcept { m_rps = rps; }

private:
    static void UpdateTransform() noexcept;
    static void CapZoomLevel() noexcept;
    static void ResetTransformation() noexcept;

private:
    static sf::Transform m_transform;
    static std::optional<sf::Vector2f *> m_follow;

    static sf::Vector2f m_position;
    static sf::Transform m_positionTransform;

    static float m_rotation;
    static sf::Transform m_rotationTransform;
    static float m_rps; // Rotations per second

    static sf::Vector2f m_zoom;
    static sf::Transform m_zoomTransform;

    static bool m_engaged;
    static sf::Vector2f m_lastPos;
};