#pragma once


class AidWidget
{
	public:
	AidWidget() = default;
	virtual ~AidWidget() = default;

	// Method to render the widget
	virtual void Render() = 0;
};