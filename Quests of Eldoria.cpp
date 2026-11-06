// ============================================================================
//  ELDORIA CLASH  -  "Quests of Eldoria"
//  Console RPG (CLO-2 & CLO-3 Capstone Project)
//
//  Team: Waleed Ahmed (093), Muhammad Saim (065), Amir Ali (011),
//        Raja Abdul Hadi (073)
//
//  Features
//   1. Character creation (name + Warrior / Mage / Rogue)
//   2. Character info screen, shown at the start of every quest
//   3. Quest board with Easy / Medium / Hard quests (chosen with a switch)
//   4. Turn-based random battle system with limited special moves
//   5. XP based levelling (health up, XP reset)
//   6. Branching storyline - your choices change the fight
//   7. Game loop with an exit option (also available mid-battle)
//   8. Inventory (array) with usable items          [extra credit]
//   9. Scoring system                               [extra credit]
// ============================================================================

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
using namespace std;

// ----------------------------------------------------------------------------
//  Constants
// ----------------------------------------------------------------------------
const int MAX_INVENTORY = 8;
const int NUM_QUESTS    = 7;

const int MEDIUM_MOVES_PER_BATTLE = 4;
const int HIGH_MOVES_PER_BATTLE   = 2;

const int POTION_HEAL = 40;
const int ELIXIR_BONUS = 6;

// Item names (kept in one place so the code never mistypes them)
const string ITEM_POTION  = "Health Potion";
const string ITEM_ELIXIR  = "Elixir of Strength";
const string ITEM_TROPHY  = "Monster Trophy";

// ----------------------------------------------------------------------------
//  Structures
// ----------------------------------------------------------------------------
struct Player {
    string name;
    string playerClass;
    int health;
    int maxHealth;
    int level;
    int experiencePoints;
    int xpToNextLevel;
    int score;
    int questsCompleted;
    string inventory[MAX_INVENTORY];   // array based inventory
    int itemCount;
};

struct Quest {
    string title;
    string difficulty;      // "Easy", "Medium", "Hard"
    string enemyName;
    string description;
    int enemyHealth;
    int enemyMinDamage;
    int enemyMaxDamage;
    int rewardXP;
    int rewardScore;
    int recommendedLevel;
    bool completed;
};

// Changes to the coming battle that are caused by the player's story choices
struct BattleMods {
    int enemyHealthChange;   // negative = enemy starts weaker
    int enemyDamageChange;   // negative = enemy hits softer
    int playerDamageBonus;   // flat bonus to every attack
    int playerHealthChange;  // immediate damage / healing before the fight
};

// ----------------------------------------------------------------------------
//  Small helpers
// ----------------------------------------------------------------------------
void line(char c = '=', int n = 60) {
    cout << string(n, c) << "\n";
}

// Random number in [lo, hi]
int randomRange(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

// Reads a whole line. If input is closed (Ctrl+Z / Ctrl+D) the game ends cleanly.
string readLine(const string& prompt) {
    string s;
    cout << prompt;
    if (!getline(cin, s)) {
        cout << "\nInput closed. Farewell, adventurer!\n";
        exit(0);
    }
    return s;
}

// Reads an integer between lo and hi (inclusive), repeating until valid.
int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        try {
            size_t used = 0;
            int v = stoi(s, &used);
            if (used == s.size() && v >= lo && v <= hi) return v;
        }
        catch (...) {}
        cout << "  Please enter a number from " << lo << " to " << hi << ".\n";
    }
}

// Reads a single menu letter in upper case ('\0' if empty input)
char readChoice(const string& prompt) {
    string s = readLine(prompt);
    if (s.empty()) return '\0';
    return (char)toupper((unsigned char)s[0]);
}

bool askYesNo(const string& prompt) {
    while (true) {
        char c = readChoice(prompt);
        if (c == 'Y') return true;
        if (c == 'N') return false;
        cout << "  Invalid input! Please enter Y or N.\n";
    }
}

// ----------------------------------------------------------------------------
//  Inventory
// ----------------------------------------------------------------------------
bool addItem(Player& p, const string& item) {
    if (p.itemCount >= MAX_INVENTORY) {
        cout << "  Your bag is full - you had to leave the " << item << " behind.\n";
        return false;
    }
    p.inventory[p.itemCount++] = item;
    cout << "  >> You found: " << item << "!\n";
    return true;
}

void removeItem(Player& p, int index) {
    for (int i = index; i < p.itemCount - 1; i++)
        p.inventory[i] = p.inventory[i + 1];
    p.itemCount--;
}

void displayInventory(const Player& p) {
    cout << "  Inventory (" << p.itemCount << "/" << MAX_INVENTORY << "): ";
    if (p.itemCount == 0) {
        cout << "empty\n";
        return;
    }
    cout << "\n";
    for (int i = 0; i < p.itemCount; i++)
        cout << "    " << (i + 1) << ". " << p.inventory[i] << "\n";
}

// ----------------------------------------------------------------------------
//  Character creation / info
// ----------------------------------------------------------------------------
void createCharacter(Player& p) {
    line();
    cout << "              CHARACTER CREATION\n";
    line();

    do {
        p.name = readLine("Enter character name: ");
    } while (p.name.empty());

    {
        cout << "\nChoose a class:\n"
             << "  1. Warrior - tough (130 HP), steady damage\n"
             << "  2. Mage    - fragile (90 HP), strong spells (+3 damage)\n"
             << "  3. Rogue   - quick (100 HP), 25% chance of a critical hit\n";
        int c = readInt("Your choice (1-3): ", 1, 3);
        switch (c) {
        case 1: p.playerClass = "Warrior"; p.maxHealth = 130; break;
        case 2: p.playerClass = "Mage";    p.maxHealth = 90;  break;
        case 3: p.playerClass = "Rogue";   p.maxHealth = 100; break;
        }
    }

    p.health = p.maxHealth;
    p.level = 1;
    p.experiencePoints = 0;
    p.xpToNextLevel = 30;
    p.score = 0;
    p.questsCompleted = 0;
    p.itemCount = 0;

    cout << "\nWelcome, " << p.name << " the " << p.playerClass << "!\n";
    // Starter kit
    addItem(p, ITEM_POTION);
}

void displayPlayerInfo(const Player& p) {
    line('-');
    cout << "  CHARACTER INFORMATION\n";
    line('-');
    cout << "  Name:     " << p.name << "\n";
    cout << "  Class:    " << p.playerClass << "\n";
    cout << "  Level:    " << p.level << "\n";
    cout << "  Health:   " << p.health << " / " << p.maxHealth << "\n";
    cout << "  XP:       " << p.experiencePoints << " / " << p.xpToNextLevel << "\n";
    cout << "  Score:    " << p.score << "\n";
    cout << "  Quests completed: " << p.questsCompleted << " / " << NUM_QUESTS << "\n";
    displayInventory(p);
    line('-');
}

// ----------------------------------------------------------------------------
//  Quest data
// ----------------------------------------------------------------------------
void initQuests(Quest q[]) {
    //            title                        diff      enemy                  description
    //            HP   minD maxD  XP  score recLvl done
    q[0] = { "The Whispering Woods", "Easy", "Goblin Raider",
             "Goblins are looting travellers on the forest road.",
             70, 3, 6, 20, 100, 1, false };
    q[1] = { "Village Under Siege", "Easy", "Shadow Wolf",
             "Civilians are under attack by monsters. Please help them!",
             90, 4, 7, 25, 150, 1, false };
    q[2] = { "Save the Children", "Medium", "Malificent the Wicked",
             "The village children are trapped in a house with a dangerous witch.",
             130, 6, 10, 40, 250, 2, false };
    q[3] = { "The Flying Dragon", "Medium", "Flying Dragon",
             "A dragon circles the sky above the market square.",
             160, 7, 12, 50, 300, 3, false };
    q[4] = { "Falkor's Lair", "Hard", "Falkor the Beast",
             "A monstrous beast has made a lair at the mountain pass.",
             200, 9, 14, 65, 450, 4, false };
    q[5] = { "Depths of the Leviathan", "Hard", "Leviathan",
             "A sea serpent is swallowing ships in the harbour.",
             240, 10, 16, 80, 550, 5, false };
    q[6] = { "The Tyrant of Eldoria", "Hard", "The Tyrant",
             "The king calls you to the castle - the head of all monsters awaits!",
             320, 12, 18, 120, 1000, 6, false };
}

void displayQuestMenu(const Quest q[]) {
    line();
    cout << "                    QUEST BOARD\n";
    line();
    for (int i = 0; i < NUM_QUESTS; i++) {
        cout << "  " << (i + 1) << ". " << q[i].title
             << "  [" << q[i].difficulty << "]"
             << "  (recommended level " << q[i].recommendedLevel << ")";
        if (q[i].completed) cout << "  -- COMPLETED";
        cout << "\n       " << q[i].description << "\n";
    }
    cout << "  0. Exit game\n";
    line();
}

// ----------------------------------------------------------------------------
//  Dynamic storyline (branching paths)
//  Each quest presents a decision; the answer changes the battle through
//  BattleMods and may hand out items.
// ----------------------------------------------------------------------------
BattleMods playStory(Player& p, const Quest& q, int questIndex) {
    BattleMods m = { 0, 0, 0, 0 };

    line('~');
    cout << "  " << q.title << " [" << q.difficulty << "]\n";
    line('~');
    cout << "In a mystical land known as Eldoria, where magic and technology coexist,\n"
         << "an ancient prophecy foretells a hero who will restore balance to the realms.\n\n";

    switch (questIndex) {
    case 0: {   // Whispering Woods
        cout << "You reach a fork in the forest. Smoke rises to the left, a ravine opens to the right.\n"
             << "  1. Follow the smoke straight into the goblin camp\n"
             << "  2. Sneak through the ravine and strike from behind\n";
        int c = readInt("Your decision: ", 1, 2);
        if (c == 1) {
            cout << "You charge in. The goblins are alert and the Raider hits hard!\n";
            m.enemyDamageChange = 2;
        } else if (randomRange(1, 100) <= 70) {
            cout << "Your stealth pays off - you surprise the Raider and find a potion in his loot!\n";
            m.enemyHealthChange = -15;
            addItem(p, ITEM_POTION);
        } else {
            cout << "A twig snaps! You slip on the ravine rocks and hurt yourself.\n";
            m.playerHealthChange = -10;
        }
        break;
    }
    case 1: {   // Village Under Siege
        cout << "A Shadow Wolf stalks the village. An old man begs for help from his burning house.\n"
             << "  1. Rescue the old man first\n"
             << "  2. Ignore the fire and hunt the wolf immediately\n";
        int c = readInt("Your decision: ", 1, 2);
        if (c == 1) {
            cout << "You save him. Grateful, he gives you an Elixir of Strength!\n";
            addItem(p, ITEM_ELIXIR);
            m.playerHealthChange = -5;
        } else {
            cout << "You catch the wolf off guard while it is distracted by the flames.\n";
            m.enemyHealthChange = -20;
        }
        break;
    }
    case 2: {   // Save the Children
        cout << "Malificent hides in the house with the children as hostages.\n"
             << "  1. Try to negotiate with her\n"
             << "  2. Break down the door and attack\n"
             << "  3. Search the cellar for another way in\n";
        int c = readInt("Your decision: ", 1, 3);
        switch (c) {
        case 1:
            if (p.level >= 2) {
                cout << "Your reputation precedes you. Malificent is shaken and weakened by doubt.\n";
                m.enemyDamageChange = -3;
            } else {
                cout << "She laughs at a novice like you and attacks with fury!\n";
                m.enemyDamageChange = 3;
            }
            break;
        case 2:
            cout << "Bang! The door shatters - the children are safe but she is enraged.\n";
            m.enemyDamageChange = 2;
            m.playerDamageBonus = 2;
            break;
        case 3:
            cout << "In the cellar you find a hidden potion and a safe tunnel behind her.\n";
            addItem(p, ITEM_POTION);
            m.enemyHealthChange = -25;
            break;
        }
        break;
    }
    case 3: {   // Flying Dragon
        cout << "The dragon swoops over the square. Archers beg you to lead them.\n"
             << "  1. Lead the archers to ground the dragon with nets\n"
             << "  2. Climb the bell tower and leap onto its back\n";
        int c = readInt("Your decision: ", 1, 2);
        if (c == 1) {
            cout << "The nets bring the dragon down hard - it starts the fight wounded.\n";
            m.enemyHealthChange = -30;
        } else if (randomRange(1, 100) <= 50) {
            cout << "A daring leap! You land on the dragon and strike a vital spot.\n";
            m.enemyHealthChange = -50;
            m.playerDamageBonus = 2;
        } else {
            cout << "You miss and crash into the stalls. Ouch!\n";
            m.playerHealthChange = -20;
        }
        break;
    }
    case 4: {   // Falkor
        cout << "Falkor's lair is ahead. You find a weathered shrine offering a blessing.\n"
             << "  1. Pray at the shrine\n"
             << "  2. Take the offering of gold and leave the blessing\n";
        int c = readInt("Your decision: ", 1, 2);
        if (c == 1) {
            cout << "A warm light heals your wounds and strengthens your arm.\n";
            m.playerHealthChange = 25;
            m.playerDamageBonus = 2;
        } else {
            cout << "The shrine's spirit curses you for your greed, but the gold gives you a reward.\n";
            p.score += 50;
            m.enemyDamageChange = 2;
        }
        break;
    }
    case 5: {   // Leviathan
        cout << "The Leviathan thrashes in the harbour. A fisherman offers help.\n"
             << "  1. Take his harpoon boat and fight from the water\n"
             << "  2. Fight from the pier with your own weapon\n";
        int c = readInt("Your decision: ", 1, 2);
        if (c == 1) {
            cout << "The harpoons bite deep before the real fight begins!\n";
            m.enemyHealthChange = -30;
            m.playerHealthChange = -10;
        } else {
            cout << "Solid ground under your feet gives you confidence.\n";
            m.playerDamageBonus = 3;
        }
        break;
    }
    case 6: {   // The Tyrant
        cout << "The King has been captured. The Tyrant sits on the throne.\n"
             << "  1. Free the King first (he joins the fight)\n"
             << "  2. Challenge the Tyrant alone\n"
             << "  3. Poison the Tyrant's wine, then attack\n";
        int c = readInt("Your decision: ", 1, 3);
        switch (c) {
        case 1:
            cout << "The freed King slashes at the Tyrant with a royal blade!\n";
            m.enemyHealthChange = -40;
            break;
        case 2:
            cout << "\"Alone? Brave... or foolish!\" The crowd cheers you on.\n";
            m.playerDamageBonus = 4;
            m.enemyDamageChange = 2;
            break;
        case 3:
            if (randomRange(1, 100) <= 60) {
                cout << "The Tyrant gulps the wine - he is poisoned!\n";
                m.enemyHealthChange = -60;
            } else {
                cout << "He notices the trick and spits it in your face!\n";
                m.playerHealthChange = -15;
                m.enemyDamageChange = 2;
            }
            break;
        }
        break;
    }
    }
    cout << "\n";
    return m;
}

// ----------------------------------------------------------------------------
//  Leveling
// ----------------------------------------------------------------------------
void gainExperience(Player& p, int xp) {
    p.experiencePoints += xp;
    cout << "  You gained " << xp << " XP.\n";

    while (p.experiencePoints >= p.xpToNextLevel) {
        p.level++;
        p.experiencePoints = 0;                      // reset XP after levelling
        p.xpToNextLevel = 30 + (p.level - 1) * 20;   // each level needs more XP
        p.maxHealth += 20;
        p.health = p.maxHealth;                      // full heal on level-up
        cout << "\n  *** LEVEL UP! You are now level " << p.level << "! ***\n"
             << "  Max health increased to " << p.maxHealth << " and you are fully healed.\n";
        // (loop ends since XP was reset to 0)
    }
}

// ----------------------------------------------------------------------------
//  Battle system
// ----------------------------------------------------------------------------
// Damage for one attack. type: 'L' low, 'M' medium, 'H' high
int calculatePlayerDamage(const Player& p, char type, int bonus) {
    int base = 6 + p.level * 2 + bonus;
    int dmg = 0;
    switch (type) {
    case 'L': dmg = base + randomRange(0, 8);   break;
    case 'M': dmg = base + randomRange(4, 14);  break;
    case 'H': dmg = base + randomRange(10, 24); break;
    }
    if (p.playerClass == "Mage") dmg += 3;
    if (p.playerClass == "Rogue" && randomRange(1, 100) <= 25) {
        dmg *= 2;
        cout << "  CRITICAL HIT!\n";
    }
    return dmg;
}

// Opens the inventory during battle. Returns true if a turn was used.
bool useItem(Player& p, int& damageBonus) {
    if (p.itemCount == 0) {
        cout << "  Your inventory is empty!\n";
        return false;
    }
    displayInventory(p);
    int c = readInt("  Use which item? (0 = cancel): ", 0, p.itemCount);
    if (c == 0) return false;

    string item = p.inventory[c - 1];
    if (item == ITEM_POTION) {
        p.health += POTION_HEAL;
        if (p.health > p.maxHealth) p.health = p.maxHealth;
        cout << "  You drink the potion and recover " << POTION_HEAL << " HP. Health: " << p.health << "\n";
        removeItem(p, c - 1);
        return true;
    }
    if (item == ITEM_ELIXIR) {
        damageBonus += ELIXIR_BONUS;
        cout << "  You drink the elixir. Your attacks deal +" << ELIXIR_BONUS << " damage this battle!\n";
        removeItem(p, c - 1);
        return true;
    }
    cout << "  The " << item << " can't be used in battle.\n";
    return false;
}

// Returns: 1 = player won, 0 = player lost, -1 = player fled / quit
int battle(Player& p, const Quest& q, const BattleMods& mods, int& damageDealtTotal) {
    int enemyHealth = q.enemyHealth + mods.enemyHealthChange;
    int enemyMin = q.enemyMinDamage + mods.enemyDamageChange;
    int enemyMax = q.enemyMaxDamage + mods.enemyDamageChange;
    if (enemyMin < 1) enemyMin = 1;
    if (enemyMax < enemyMin) enemyMax = enemyMin;
    int damageBonus = mods.playerDamageBonus;
    int mediumLeft = MEDIUM_MOVES_PER_BATTLE;
    int highLeft = HIGH_MOVES_PER_BATTLE;
    int turn = 1;

    line('=');
    cout << "  BATTLE: " << p.name << " vs " << q.enemyName << "!\n";
    line('=');

    while (enemyHealth > 0 && p.health > 0) {
        cout << "\n--- Turn " << turn << " ---   "
             << p.name << ": " << p.health << "/" << p.maxHealth << " HP   |   "
             << q.enemyName << ": " << enemyHealth << " HP\n"
             << "  [L] Light attack\n"
             << "  [M] Medium attack  (" << mediumLeft << " left)\n"
             << "  [H] Heavy attack   (" << highLeft << " left)\n"
             << "  [I] Use item\n"
             << "  [F] Flee the battle\n"
             << "  [Q] Quit the game\n";
        char c = readChoice("Action: ");

        int dmg = 0;
        switch (c) {
        case 'L':
            dmg = calculatePlayerDamage(p, 'L', damageBonus);
            break;
        case 'M':
            if (mediumLeft <= 0) { cout << "  You have exhausted your medium moves!\n"; continue; }
            mediumLeft--;
            dmg = calculatePlayerDamage(p, 'M', damageBonus);
            break;
        case 'H':
            if (highLeft <= 0) { cout << "  You have exhausted your heavy moves!\n"; continue; }
            highLeft--;
            dmg = calculatePlayerDamage(p, 'H', damageBonus);
            break;
        case 'I':
            if (!useItem(p, damageBonus)) continue;   // cancelled: no turn used
            break;
        case 'F':
            if (randomRange(1, 100) <= 60) {
                cout << "  You escape from the battle!\n";
                return -1;
            }
            cout << "  You failed to escape!\n";
            break;
        case 'Q':
            if (askYesNo("  Quit the game? Progress will be lost (Y/N): ")) {
                cout << "\nThanks for playing Quests of Eldoria. Final score: " << p.score << "\n";
                exit(0);
            }
            continue;
        default:
            cout << "  Invalid input!\n";
            continue;
        }

        // ---- player's strike ----
        if (dmg > 0) {
            enemyHealth -= dmg;
            damageDealtTotal += dmg;
            cout << "  You hit " << q.enemyName << " for " << dmg << " damage!\n";
            if (enemyHealth <= 0) break;
        }

        // ---- enemy's strike ----
        int enemyDmg = randomRange(enemyMin, enemyMax);
        // 10% chance the enemy lands a heavy blow
        if (randomRange(1, 100) <= 10) {
            enemyDmg += enemyMax / 2;
            cout << "  " << q.enemyName << " lands a savage blow!\n";
        }
        p.health -= enemyDmg;
        if (p.health < 0) p.health = 0;
        cout << "  " << q.enemyName << " hits you for " << enemyDmg << " damage!\n";
        turn++;
    }

    return (enemyHealth <= 0) ? 1 : 0;
}

// ----------------------------------------------------------------------------
//  Quest flow
// ----------------------------------------------------------------------------
void embarkOnQuest(Player& p, Quest& q, int questIndex) {
    displayPlayerInfo(p);            // required: show info at the beginning of each quest

    BattleMods mods = playStory(p, q, questIndex);
    p.health += mods.playerHealthChange;
    if (p.health > p.maxHealth) p.health = p.maxHealth;
    if (p.health < 1) p.health = 1;  // story hazards never kill outright

    int damageDealt = 0;
    int result = battle(p, q, mods, damageDealt);

    cout << "\n";
    if (result == 1) {
        q.completed = true;
        p.questsCompleted++;

        // Score = quest reward + bonus for health left + bonus for damage dealt
        int healthBonus = p.health * 2;
        int performance = damageDealt / 5;
        int gained = q.rewardScore + healthBonus + performance;
        p.score += gained;

        line('*');
        cout << "  VICTORY! You defeated " << q.enemyName << "!\n";
        line('*');
        cout << "  Score gained: " << gained << " (reward " << q.rewardScore
             << " + health bonus " << healthBonus << " + performance " << performance << ")\n";
        gainExperience(p, q.rewardXP);

        // Loot
        if (randomRange(1, 100) <= 50) addItem(p, ITEM_POTION);
        if (randomRange(1, 100) <= 30) addItem(p, ITEM_TROPHY);
    }
    else if (result == 0) {
        line('!');
        cout << "  DEFEATED! " << q.enemyName << " was too strong...\n";
        line('!');
        int lost = p.score >= 50 ? 50 : p.score;
        p.score -= lost;
        p.experiencePoints = 0;
        p.health = p.maxHealth / 2;
        cout << "  You wake up in the village healer's house with " << p.health << " HP.\n"
             << "  You lost " << lost << " score and your current XP. Train and try again!\n";
    }
    else {
        cout << "  You return to the village. The quest is still waiting...\n";
        p.score -= (p.score >= 20) ? 20 : p.score;
    }
}

// Heal a little between quests so the player is not stuck with 1 HP forever
void rest(Player& p) {
    int heal = p.maxHealth / 4;
    p.health += heal;
    if (p.health > p.maxHealth) p.health = p.maxHealth;
    cout << "  You rest at the village inn and recover " << heal << " HP.\n";
}

bool allQuestsDone(const Quest q[]) {
    for (int i = 0; i < NUM_QUESTS; i++)
        if (!q[i].completed) return false;
    return true;
}

// ----------------------------------------------------------------------------
//  Main / game loop
// ----------------------------------------------------------------------------
int main() {
    srand((unsigned)time(NULL));   // seed the RNG so every game is different

    Player player;
    Quest quests[NUM_QUESTS];
    initQuests(quests);

    line('#');
    cout << "              ELDORIA CLASH\n"
         << "             Quests of Eldoria\n";
    line('#');

    createCharacter(player);

    bool running = true;
    while (running) {
        cout << "\n";
        displayQuestMenu(quests);
        int choice = readInt("Choose a quest (0 to exit): ", 0, NUM_QUESTS);

        switch (choice) {
        case 0:
            running = false;
            break;
        case 1: case 2: case 3: case 4: case 5: case 6: case 7: {
            int idx = choice - 1;
            if (quests[idx].completed) {
                cout << "  You have already completed that quest.\n";
                break;
            }
            embarkOnQuest(player, quests[idx], idx);
            if (allQuestsDone(quests)) {
                line('#');
                cout << "  CONGRATULATIONS " << player.name << "! You have saved Eldoria\n"
                     << "  and become the legendary Elemental Master!\n";
                line('#');
                running = false;
            } else {
                rest(player);
            }
            break;
        }
        }
    }

    cout << "\n";
    displayPlayerInfo(player);
    cout << "Thanks for playing Quests of Eldoria! Final score: " << player.score << "\n";
    return 0;
}
