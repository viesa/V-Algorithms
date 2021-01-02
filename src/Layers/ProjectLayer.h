﻿#pragma once

#include "Layers/BaseLayer.h"

#include "AlgorithmManager.h"

namespace Se
{
class ProjectLayer : public BaseLayer
{
public:
	void OnAttach(std::shared_ptr<BatchLoader> &loader) override;
	void OnDetach() override;

	void OnUpdate() override;
	void OnGuiRender() override;

	void OnRenderTargetResize(const sf::Vector2f &newSize) override;

private:
	std::shared_ptr<AlgorithmManager> _algorithmManager;

    bool _showSystemWindows = false;
	bool _showDemoWindow = false;

};
}