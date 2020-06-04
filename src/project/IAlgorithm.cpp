#include "IAlgorithm.h"

IAlgorithm::IAlgorithm()
    : m_elements(10000),
      m_sleepDelay(sf::seconds(0.01f)),
      m_state(State::WaitingForStart),
      m_isActive(true),
      m_minorDelay(false),
      m_visType(VisType::Bars)
{
    m_name.setFont(*FontMgr::Get("res/arial.ttf"));
}

void IAlgorithm::Draw(const sf::FloatRect &rect)
{
    switch (m_visType)
    {
    case VisType::Bars:
    {
        DrawBars(rect);
        break;
    }
    case VisType::Circles:
    {
        DrawCircles(rect);
        break;
    }
    case VisType::Hoops:
    {
        DrawHoops(rect);
        break;
    }
    default:
        break;
    }
}

void IAlgorithm::DrawName(const sf::FloatRect &rect)
{
    m_name.setString(GetName());
    m_name.setPosition(rect.left + rect.width / 2.0f, rect.top - 35.0f);
    Camera::DrawText(m_name, TextAlign::Middle);
}

void IAlgorithm::Start()
{
    if (m_state == State::WaitingForStart)
    {
        CollectSorter();
        m_state = State::Sorting;
        m_sorter = std::thread(SortThreadFn, this);
    }
}

void IAlgorithm::Restart()
{
    if (m_state == State::Sorting || m_state == State::Paused || m_state == State::Finished)
    {
        CollectSorter();
        m_state = State::WaitingForStart;
        m_elements = m_elementsRestart;
    }
}

void IAlgorithm::Pause()
{
    if (m_state == State::Sorting)
        m_state = State::Paused;
}

void IAlgorithm::Resume()
{
    if (m_state == State::Paused)
        m_state = State::Sorting;
}

void IAlgorithm::Reset()
{
    CollectSorter();
    m_state = State::WaitingForStart;
    m_elements = m_elementsReset;
}

void IAlgorithm::Resize(size_t size)
{
    if (m_elements.size() == size)
        return;

    m_elements.clear();
    m_elementsRestart.clear();
    m_elementsReset.clear();
    m_elements.resize(size);
    m_elementsRestart.resize(size);
    m_elementsReset.resize(size);

    long n = 1;
    std::generate(m_elements.begin(), m_elements.end(), [&n] {
        return Element(n++);
    });
    m_elementsRestart = m_elements;
    m_elementsReset = m_elements;
}

void IAlgorithm::PopPushUntil(size_t size)
{
    CollectSorter();
    while (m_elementsReset.size() != size)
    {
        if (size < m_elementsReset.size())
            m_elementsReset.pop_back();
        else if (size > m_elementsReset.size())
            m_elementsReset.push_back(Element(m_elementsReset.size() + 1));
    }
    m_elements = m_elementsReset;
    m_elementsRestart = m_elementsReset;
}

void IAlgorithm::Shuffle(std::mt19937 generator)
{
    Reset();
    std::shuffle(m_elementsRestart.begin(), m_elementsRestart.end(), generator);
    m_elements = m_elementsRestart;
}

void IAlgorithm::SetSleepDelay(sf::Time delay) noexcept
{
    m_sleepDelay = delay;
    m_minorDelay = (m_sleepDelay.asMicroseconds() < 1000);
}

void IAlgorithm::PauseCheck()
{
    while (m_state == State::Paused && m_state != State::BeingCollected)
        sf::sleep(sf::seconds(0.01f));
}

void IAlgorithm::SleepDelay()
{
    if (!m_minorDelay)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(m_sleepDelay.asMicroseconds()));
    }
    else
    {
        m_minorDelayTimer += m_sleepDelay.asMicroseconds();
        while (m_minorDelayTimer > 1000)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
            m_minorDelayTimer -= 1000;
        }
    }
}

void IAlgorithm::SortThreadFn()
{
    Sort();
    if (m_state != State::BeingCollected)
        OnFinish();
}

void IAlgorithm::OnFinish()
{
    for (auto &bar : m_elements)
        bar.color = sf::Color::Green;
    m_state = State::Finished;
}

void IAlgorithm::CollectSorter()
{
    auto tmpState = m_state;
    m_state = State::BeingCollected;
    if (m_sorter.joinable())
        m_sorter.join();
    m_state = tmpState;
}

void IAlgorithm::DrawBars(const sf::FloatRect &rect)
{
    sf::RectangleShape shape;
    sf::Vector2f size(rect.width / m_elements.size(), 0.0f);
    sf::Vector2f positionOffset(rect.left, rect.top);
    if (size.x > 2.0f && size.y > 2.0f)
    {
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color(150, 150, 150));
    }
    float heightMult = rect.height / m_elements.size();

    bool spectrumColorsForThisLoop = m_usingSpectrumColors;

    for (size_t i = 0; i < m_elements.size(); i++)
    {
        size.y = m_elements[i].value * heightMult;
        shape.setFillColor(GetElementColor(i));
        shape.setPosition(positionOffset.x + size.x * static_cast<float>(i), positionOffset.y + rect.height - size.y);
        shape.setSize(size);
        Camera::Draw(shape);
    }
}

void IAlgorithm::DrawCircles(const sf::FloatRect &rect)
{
    sf::VertexArray vertexArray(sf::TriangleFan, 1 + 2 * m_elements.size());
    float maxRadius = static_cast<float>(std::min(rect.width / 2, rect.height / 2));
    float maxValue = m_elements.size();
    float angleDelta = 2.0f * PI<> / (m_elements.size() * 2.0f);
    float heightMult = maxRadius / m_elements.size();
    sf::Vector2f rectMid = Lib::Mid(rect);

    vertexArray[0] = sf::Vertex(rectMid, sf::Color(255, 255, 255));
    for (size_t i = 0; i < m_elements.size() * 2; i += 2)
    {
        sf::Vector2f line0 = vl::Rotate(sf::Vector2f(0.0f, -m_elements[i / 2].value * heightMult) + rectMid, angleDelta * static_cast<float>(i), rectMid);
        sf::Vector2f line1 = vl::Rotate(sf::Vector2f(0.0f, -m_elements[i / 2].value * heightMult) + rectMid, angleDelta * static_cast<float>(i + 1), rectMid);
        vertexArray[i + 1] = sf::Vertex(line0, GetElementColor(i / 2));
        vertexArray[i + 2] = sf::Vertex(line1, GetElementColor(i / 2));
    }
    Camera::Draw(vertexArray);
}

void IAlgorithm::DrawHoops(const sf::FloatRect &rect)
{
    sf::CircleShape circle;
    circle.setFillColor(sf::Color(0.0f, 0.0f, 0.0f, 0.0f));
    circle.setOutlineThickness(1);
    float radiusMult = rect.height / m_elements.size() / 3.0f;
    sf::Vector2f rectMid = Lib::Mid(rect);
    for (size_t i = 0; i < m_elements.size(); i++)
    {
        float radius = m_elements[i].value * radiusMult;
        sf::Color color = GetElementColor(i);
        color.a = 50;
        circle.setRadius(radius);
        circle.setOutlineColor(color);
        circle.setPosition(rectMid.x - radius, rect.top + radiusMult * static_cast<float>(i));
        Camera::Draw(circle);
    }
}

sf::Color IAlgorithm::GetElementColor(size_t index)
{
    if (m_usingSpectrumColors)
        Lib::ValueToSpectrum(m_elements[index].value, (long)m_elements.size());
    else
        return m_elements[index].color;
}