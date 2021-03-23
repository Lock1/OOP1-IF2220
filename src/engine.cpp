// 13519214
#include "header/render.hpp"
#include "header/engine.hpp"
#include "header/playerinput.hpp"
#include "header/entities/map.hpp"
#include "header/entities/player.hpp"
#include "header/entities/engimon.hpp"
#include "header/entities/species.hpp"
#include "header/skilldatabase.hpp"
#include "header/speciesdatabase.hpp"
#include "header/entities/attributes/elementtype.hpp"
#include "header/inventory"
#include <iostream>
#include <string>
#include <chrono>       // Time and tick system
#include <thread>       // For sleep()
#include <map>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>

using namespace std;


Engine::Engine() : messageList(MAX_MESSAGE, MSG_MAX_X), statMessage(MAX_MESSAGE-10, MSG_MAX_X-5),
        thisisfine(MAX_MESSAGE-15, MSG_MAX_X-5), // DEBUG
        player(MAX_INVENTORY, MAX_SKILL_ID), maxSkillID(MAX_SKILL_ID),
        // map(MAP_MAX_X, MAP_MAX_Y, SEA_STARTING_X, SEA_STARTING_Y), // DEBUG
        map("../other/mapfile.txt"),
        userInput(INPUT_BUFFER_COUNT, INPUT_DELAY_MS), wildEngimonSpawnProbability(4), entitySpawnLimit(20),
        renderer(map, messageList), statRenderer(statMessage), ok(thisisfine) {
    // Internal variable setup
    srand((unsigned) time(NULL));
    isEngineRunning = true;
    isCommandMode = false;
    renderer.setMapOffset(MAP_OFFSET_X, MAP_OFFSET_Y);
    renderer.setMessageBoxOffset(MESSAGE_OFFSET_X, MESSAGE_OFFSET_Y);
    renderer.setCursorRestLocation(CURSOR_REST_X, CURSOR_REST_Y);

    statRenderer.setMessageBoxOffset(MESSAGE_OFFSET_X+messageList.getMaxStringLength()+3, MESSAGE_OFFSET_Y);
    statRenderer.setCursorRestLocation(CURSOR_REST_X, CURSOR_REST_Y);

    // DEBUG
    ok.setMessageBoxOffset(MESSAGE_OFFSET_X+messageList.getMaxStringLength()+3, MESSAGE_OFFSET_Y+13);
    ok.setCursorRestLocation(CURSOR_REST_X, CURSOR_REST_Y);

    renderer.setMessageTitle("Ini kotak gan");
    statRenderer.setMessageTitle("Ini bukan kotak");
    ok.setMessageTitle("Ini bo'ongan"); // <<< DEBUG

    // TODO : Add prompt (?)
    // TODO : Add splash screen (?)
    try {
        skillDB.loadSkillDatabase("../other/skilldb.txt");
    }
    catch (string e) {
        cout << "Skill database not found\n";
    }

    try {
        speciesDB.loadSpeciesDatabase("../other/speciesdb.txt", skillDB);
    }
    catch (string e) {
        cout << "Species database not found\n";
    }
}

Engine::~Engine() {
    // TODO : Cleanup variable
    // TODO : Destroy allocated engimonList
}

void Engine::clearConsoleInputBuffer() {
    // Source : http://cplusplus.com/forum/beginner/248262/
    PINPUT_RECORD ClearingVar1 = new INPUT_RECORD[256];
    DWORD ClearingVar2;
    ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE),ClearingVar1,256,&ClearingVar2);
    delete[] ClearingVar1;
}


void Engine::startGame() {
    // TODO : Put game here
    system(CLEAR_SCREEN_CMD);

    map.setTileEntity(player.getPos(), &player);
    // DEBUG
    Engimon *starterEngimon = new Engimon(speciesDB.getSpecies(4), false, Position(0, 0));
    engimonList.push_back(starterEngimon);
    player.changeEngimon(starterEngimon);
    player.addEngimonItem(starterEngimon);
    map.setTileEntity(starterEngimon->getPos(), starterEngimon);


    userInput.startReadInput();
    updateCurrentEngimonMessageStatus();

    while (isEngineRunning) {
        // Drawing map and message box
        renderer.drawMap(map);
        renderer.drawMessageBox(messageList);
        statRenderer.drawMessageBox(statMessage);
        ok.drawMessageBox(thisisfine);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (evaluteInput() && not isCommandMode) {
            evaluteTick();
        }
        else if (isCommandMode) {
            commandMode();
            // Call command mode
        }

    }
    userInput.stopReadInput();
}

bool Engine::evaluteInput() {
    InputType inputKey = userInput.getUserInput();
    Tile targetTile = Tile(0, 0, Grass);
    bool isMovementValid = false;
    Direction targetDirection;
    switch (inputKey) {
        case EscKey:
            isEngineRunning = false;
            break;
        case Up:
            // TODO : Interaction completion
            if (player.getPos().getY() > 0) {
                targetTile = map.getTileAt(player.getPos().getX(), player.getPos().getY()-1);
                if (player.isMoveLocationValid(targetTile)) {
                    targetDirection = North;
                    isMovementValid = true;
                }
            }
            break;
        case Down:
            if (player.getPos().getY() < map.getSizeY() - 1) {
                targetTile = map.getTileAt(player.getPos().getX(), player.getPos().getY()+1);
                if (player.isMoveLocationValid(targetTile)) {
                    targetDirection = South;
                    isMovementValid = true;
                }
            }
            break;
        case Left:
            if (player.getPos().getX() > 0) {
                targetTile = map.getTileAt(player.getPos().getX()-1, player.getPos().getY());
                if (player.isMoveLocationValid(targetTile)) {
                    targetDirection = West;
                    isMovementValid = true;
                }
            }
            break;
        case Right:
            if (player.getPos().getX() < map.getSizeX() - 1) {
                targetTile = map.getTileAt(player.getPos().getX()+1, player.getPos().getY());
                if (player.isMoveLocationValid(targetTile)) {
                    targetDirection = East;
                    isMovementValid = true;
                }
            }
            break;
        case KeyboardE:
            isCommandMode = true;
            break;
    }

    if (isMovementValid) {
        map.moveEntity(player.getPos(), targetDirection);
        map.moveEntity(player.getCurrentEngimon()->getPos(), player.getLastDirection());
        player.getLastDirectionRef() = targetDirection;
        return true;
    }
    else if (targetTile.getEntity() != NULL) {
        Entity* targetEntity = targetTile.getEntity();
        if (targetEntity->getEntityID() == EntityEngimon) {
            Engimon *targetEngimon = (Engimon *) targetEntity;
            if (not targetEngimon->isWildEngimon()) {
                string interactionMessage = targetEngimon->getEngimonName();
                interactionMessage = interactionMessage + " : ";
                interactionMessage = interactionMessage + targetEngimon->getInteractString();
                messageList.addMessage(interactionMessage);
            }
        }
    }

    return false;
}

void Engine::evaluteTick() {
    // TODO : Add here
    map.wildEngimonRandomMove();
    unsigned int randomNumber = rand() % 100;
    if (Entity::getEntityCount() < entitySpawnLimit && randomNumber < wildEngimonSpawnProbability) {
        unsigned int randomSpeciesID = (rand() % (speciesDB.getSpeciesCount() - 1)) + 1;
        // TODO : Extra, fix mod operator
        engimonList.push_back(map.spawnWildEngimon(speciesDB.getSpecies(randomSpeciesID)));
    }
}

void Engine::commandMode() {
    userInput.toggleReadInput();
    // Temporary stop input thread from queueing movement input
    clearConsoleInputBuffer();
    // Clearing current input buffer (GetKeyState() does not clear buffer)
    string commandBuffer;
    cout << ">>> ";
    getline(cin, commandBuffer);
    cout << endl << commandBuffer << endl; // TODO : Add
    if (commandBuffer == "dbg") { // DEBUG
        player.addEngimonItem(new Engimon(speciesDB.getSpecies(3), false, Position(0, 0)));
        player.addSkillItem(4);
        player.addSkillItem(3);
        player.addSkillItem(7);
    }
    // TODO : Add
    // else if (commandBuffer == "breed")
    else if (commandBuffer == "engimon") {
        // TODO : Print everything + parent
        list<EngimonItem> engimonInv = player.getEngimonInventory();
        for (auto it = engimonInv.begin(); it != engimonInv.end(); ++it) {
            Engimon *targetEngimon = *it;
            string speciesNameMsg = "Species | ";
            speciesNameMsg = speciesNameMsg + targetEngimon->getName();
            messageList.addMessage(speciesNameMsg);

            string nameMsg = "Name    | ";
            nameMsg = nameMsg + targetEngimon->getEngimonName();
            messageList.addMessage(nameMsg);

            string levelMsg = "Lvl     | ";
            levelMsg = levelMsg + to_string(targetEngimon->getLevel());
            messageList.addMessage(levelMsg);

            string xpMsg = "XP      | ";
            xpMsg = xpMsg + to_string(targetEngimon->getXP());
            messageList.addMessage(xpMsg);

            set<ElementType> elements = targetEngimon->getElements();
            string typeMsg = "Type    | ";
            if (elements.find(Fire) != elements.end())
            typeMsg = typeMsg + "Fire ";
            else if (elements.find(Ice) != elements.end())
            typeMsg = typeMsg + "Ice ";
            else if (elements.find(Water) != elements.end())
            typeMsg = typeMsg + "Water ";
            else if (elements.find(Ground) != elements.end())
            typeMsg = typeMsg + "Ground ";
            else if (elements.find(Electric) != elements.end())
            typeMsg = typeMsg + "Electric ";
            messageList.addMessage(typeMsg);

            messageList.addMessage("");

        }
    }
    else if (commandBuffer == "item") {
        messageList.addMessage("ID Name    Count  Type");
        std::map<SkillItem,int> skillInv = player.getSkillInventory();
        for (int i = 0; i < maxSkillID; i++) {
            if (skillInv[i] > 0) {
                string skillRow;
                Skill target = skillDB.getSkill(i);
                skillRow = to_string(target.getSkillID())+ " " + target.getSkillName() + " " + to_string(skillInv[i]) +" ";
                switch (target.getSkillElement()) {
                    case Fire:
                        skillRow = skillRow + "Fire ";
                        break;
                    case Ice:
                        skillRow = skillRow + "Ice ";
                        break;
                    case Water:
                        skillRow = skillRow + "Water ";
                        break;
                    case Ground:
                        skillRow = skillRow + "Ground ";
                        break;
                    case Electric:
                        skillRow = skillRow + "Electric ";
                        break;
                }
                messageList.addMessage(skillRow);
            }
        }
        // TODO : Use item
    }
    // else if (commandBuffer == "change")
    // else if (commandBuffer == "detail")

    // getline(cin, commandBuffer);
    userInput.toggleReadInput();
    renderer.clearCursorRestArea();
    isCommandMode = false;
}

void Engine::updateCurrentEngimonMessageStatus() {
    statMessage.clearMessage();
    string speciesNameMsg = "Species | ";
    speciesNameMsg = speciesNameMsg + player.getCurrentEngimon()->getName();
    statMessage.addMessage(speciesNameMsg);

    string nameMsg = "Name    | ";
    nameMsg = nameMsg + player.getCurrentEngimon()->getEngimonName();
    statMessage.addMessage(nameMsg);

    string levelMsg = "Lvl     | ";
    levelMsg = levelMsg + to_string(player.getCurrentEngimon()->getLevel());
    statMessage.addMessage(levelMsg);

    string xpMsg = "XP      | ";
    xpMsg = xpMsg + to_string(player.getCurrentEngimon()->getXP());
    statMessage.addMessage(xpMsg);

    set<ElementType> elements = player.getCurrentEngimon()->getElements();
    string typeMsg = "Type    | ";
    if (elements.find(Fire) != elements.end())
        typeMsg = typeMsg + "Fire ";
    else if (elements.find(Ice) != elements.end())
        typeMsg = typeMsg + "Ice ";
    else if (elements.find(Water) != elements.end())
        typeMsg = typeMsg + "Water ";
    else if (elements.find(Ground) != elements.end())
        typeMsg = typeMsg + "Ground ";
    else if (elements.find(Electric) != elements.end())
        typeMsg = typeMsg + "Electric ";
    statMessage.addMessage(typeMsg);
    // TODO : Parent Check, after breeding

    statRenderer.drawMessageBox(statMessage);
}
