#pragma once
#include "Item.hpp"

class FireFlower : public Item {
public:
    FireFlower(Physics& physics, float x, float y);
    
    void update(float dt) override;
    
    bool isFireFlower() const { return true; }
};
