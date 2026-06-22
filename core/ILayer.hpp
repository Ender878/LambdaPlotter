#pragma once

class ILayer {
public:
    virtual ~ILayer() = default;

    virtual void on_update(double dt) = 0;
    virtual void on_render() = 0;
};
