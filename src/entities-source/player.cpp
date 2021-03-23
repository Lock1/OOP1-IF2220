// Player class
#include "../header/entities/player.hpp"
#include "../header/entities/entity.hpp"
#include "../header/entities/engimon.hpp"
#include <iostream>
#include <map>
#include <list>

using namespace std;

Player::Player(unsigned maxInv, unsigned maxSkillID) : Entity(1, 0, EntityPlayer, PLAYER_CHAR), lastDirection(East),
        engimonInventory(maxInv), skillInventory(maxInv, maxSkillID), maxInventorySize(maxInv) {
    currentEngimon = NULL;
}

Player::~Player() {
    // TODO : Maybe not needed
}

void Player::changeEngimon(Engimon *targetEngimon) {
    if (currentEngimon != NULL)
        targetEngimon->getPosRef() = currentEngimon->getPos();
    currentEngimon = targetEngimon;
}

Engimon* Player::getCurrentEngimon() {
    return currentEngimon;
}

bool Player::isMoveLocationValid(Tile& target) {
    if (target.getEntity() == NULL)
        return true;
    else
        return false;
}

Direction Player::getLastDirection() {
    return lastDirection;
}

Direction& Player::getLastDirectionRef() {
    return lastDirection;
}

bool Player::addSkillItem(int skillID) {
    if (skillInventory.getCurrentSize() + engimonInventory.getCurrentSize() < (int) maxInventorySize) {
        skillInventory.addItem(skillID);
        return true;
    }
    else
        return false;
}

bool Player::addEngimonItem(EngimonItem engimon) {
    if (skillInventory.getCurrentSize() + engimonInventory.getCurrentSize() < (int) maxInventorySize) {
        engimonInventory.addItem(engimon);
        return true;
    }
    else
        return false;
}

map<SkillItem, int> Player::getSkillInventory() {
    return skillInventory.getItemList();
}

list<EngimonItem> Player::getEngimonInventory() {
    return engimonInventory.getItemList();
}
