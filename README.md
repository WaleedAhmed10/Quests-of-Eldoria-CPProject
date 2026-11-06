# Eldoria Clash - "Quests of Eldoria"

Console RPG in C++ (CLO-2 & CLO-3 capstone), made by Waleed Ahmed (093), Muhammad Saim (065), Amir Ali (011), Raja Abdul Hadi (073).

## Build & run
- Visual Studio: open `Project2.sln` and run.
- g++: `g++ -std=c++17 "Quests of Eldoria.cpp" -o eldoria && ./eldoria`

## Features (mapped to the assignment)
| # | Requirement | Where |
|---|-------------|-------|
| 1 | Character creation (name, Warrior/Mage/Rogue, health + XP) | `createCharacter` |
| 2 | Character info, shown at the start of every quest | `displayPlayerInfo` |
| 3 | Quest system with Easy / Medium / Hard quests | `Quest` struct, `initQuests`, `displayQuestMenu` |
| 4 | Turn-based battle with random damage, limited special moves, flee | `battle`, `calculatePlayerDamage` |
| 5 | XP levelling: more max health, full heal, XP reset | `gainExperience` |
| 6 | Branching storyline - choices change the fight | `playStory` |
| 7 | Quest choice by number using `switch` | `main` |
| 8 | Game loop, exit from menu or mid-battle (Q) | `main`, `battle` |
| 9 | Inventory array (potions, elixirs, trophies) - extra credit | `addItem`, `useItem` |
| + | Scoring system (reward + health left + damage dealt) - extra credit | `embarkOnQuest` |

Class perks: Warrior = most HP, Mage = +3 damage, Rogue = 25% critical hit chance.
