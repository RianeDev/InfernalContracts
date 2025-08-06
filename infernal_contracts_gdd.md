# Infernal Contracts: Card Game Starter Kit
## Game Design Document v1.0

---

## Executive Summary

**Infernal Contracts** is a roguelike deck-building card game starter kit for Unreal Engine. Players descend through the Nine Circles of Hell, collecting demonic minions through "soul contracts" and battling increasingly powerful demons. The game combines the addictive progression of Slay the Spire with hellish theming perfect for tutorial content.

---

## Core Game Loop

1. **Enter Hell Layer** - Player spawns with starting deck
2. **Explore Encounters** - Choose paths through randomly generated encounters
3. **Battle Demons** - Turn-based card combat against AI enemies
4. **Collect Contracts** - Gain new minion cards after victories
5. **Boss Battle** - Face the Circle's ruling demon
6. **Descend Deeper** - Progress to next layer with upgraded deck
7. **Repeat** until all Nine Circles conquered or player is defeated

---

## Game Mechanics

### Card System
- **Minion Cards**: Creatures that attack, defend, or provide utility
- **Contract Cards**: Special abilities that manipulate the battlefield
- **Soul Cards**: Resource cards that provide energy/mana
- **Curse Cards**: Negative effects added to deck as penalties

### Combat System
- **Turn-based**: Player goes first, then AI
- **Energy System**: Start with 3 energy, gain 1 per turn (max 10)
- **Health**: Player starts with 100 HP, loses on reaching 0
- **Hand Management**: Draw 5 cards per turn, max hand size 10

### Progression System
- **Deck Building**: Add 1-2 cards after each battle
- **Card Upgrades**: Enhance existing cards at special shrines
- **Artifacts**: Permanent passive bonuses found in treasure rooms
- **Soul Currency**: Spend at shops between battles

---

## The Nine Circles of Hell

### Circle 1: Limbo (Tutorial Layer)
- **Boss**: Minos, Judge of the Damned
- **Theme**: Introduction to basic mechanics
- **Enemies**: Lost Souls, Unbaptized Spirits
- **Rewards**: Basic minion contracts

### Circle 2: Lust
- **Boss**: Asmodeus, Prince of Lust
- **Theme**: Charm and manipulation effects
- **Enemies**: Succubi, Incubi, Lustful Spirits
- **Rewards**: Charm-based contract cards

### Circle 3: Gluttony
- **Boss**: Beelzebub, Lord of Flies
- **Theme**: Consumption and growth mechanics
- **Enemies**: Cerberus, Gluttonous Shades
- **Rewards**: Cards that grow stronger when consuming others

### Circle 4: Greed
- **Boss**: Mammon, Prince of Greed
- **Theme**: Resource manipulation and hoarding
- **Enemies**: Greedy Merchants, Hoarder Demons
- **Rewards**: Economy-based contract cards

### Circle 5: Wrath
- **Boss**: Satan (as Wrath), The Furious
- **Theme**: Aggressive combat and berserker effects
- **Enemies**: Wrathful Fighters, Rage Demons
- **Rewards**: High-damage, high-risk combat cards

### Circle 6: Heresy
- **Boss**: Belphegor, Prince of Sloth/Heresy
- **Theme**: Corruption and anti-divine magic
- **Enemies**: Heretic Spirits, Fallen Angels
- **Rewards**: Cards that corrupt opponent's deck

### Circle 7: Violence
- **Boss**: Minotaur, Guardian of Violence
- **Theme**: Direct damage and battlefield control
- **Enemies**: Centaurs, Harpies, Violent Souls
- **Rewards**: Area-of-effect and control cards

### Circle 8: Fraud
- **Boss**: Geryon, Personification of Fraud
- **Theme**: Deception, card stealing, illusions
- **Enemies**: Devils, Fraudulent Spirits, Seducers
- **Rewards**: Stealth and manipulation cards

### Circle 9: Treachery
- **Boss**: Lucifer, The Fallen Angel
- **Theme**: Ultimate challenge, betrayal mechanics
- **Enemies**: Traitors, Fallen Seraphim, Ice Demons
- **Rewards**: Legendary contract cards

---

## Technical Architecture

### Core Classes (C++)

#### FCard
```cpp
struct FCard
{
    int32 ID;
    FString Name;
    FString Description;
    int32 Cost;
    int32 Attack;
    int32 Health;
    ECardType Type;
    TArray<ECardEffect> Effects;
    UTexture2D* CardArt;
};
```

#### FBattleState
```cpp
struct FBattleState
{
    TArray<FCard> PlayerHand;
    TArray<FCard> PlayerDeck;
    TArray<FCard> EnemyField;
    int32 PlayerHealth;
    int32 PlayerEnergy;
    int32 TurnNumber;
    bool bPlayerTurn;
};
```

### Blueprint Node Categories

#### Card Management Nodes
- **Create Card**: Generate new card with specified stats
- **Add Card to Deck**: Insert card into player's collection
- **Draw Cards**: Move cards from deck to hand
- **Play Card**: Execute card effects and remove from hand

#### Battle System Nodes
- **Start Battle**: Initialize combat with specified enemy
- **Process Turn**: Handle player/AI turn logic
- **Apply Damage**: Reduce health with damage calculation
- **Check Win Condition**: Determine if battle is complete

#### Progression Nodes
- **Complete Layer**: Progress to next Circle of Hell
- **Add Contract Reward**: Present card choices after victory
- **Save Progress**: Persist player's current state
- **Load Game**: Restore saved progression state

---

## Content Creation Pipeline

### Card Creation Workflow
1. **Design Phase**: Create card concept with stats and effects
2. **Art Integration**: Import card artwork (512x512 recommended)
3. **Blueprint Setup**: Use "Create Card" node with parameters
4. **Effect Programming**: Implement special abilities via Blueprint events
5. **Balancing**: Playtesting and stat adjustment

### Enemy AI Behavior
- **Simple AI**: Random card selection with basic targeting
- **Intermediate AI**: Evaluates threat levels and prioritizes targets
- **Advanced AI**: Combo-aware with strategic planning (Premium only)

### Boss Encounter Design
- **Phase-based Combat**: Bosses change behavior at health thresholds
- **Unique Mechanics**: Each boss introduces new gameplay elements
- **Escalating Difficulty**: Later bosses require specific strategies

---

## Free vs Premium Features

### Free Plugin Includes:
- Complete card framework (C++ backend)
- All Blueprint nodes for basic functionality
- First 3 Circles of Hell (Limbo, Lust, Gluttony)
- 30 base minion cards
- 10 contract cards
- Basic AI system
- Tutorial integration

### Premium Plugin Adds:
- Remaining 6 Circles of Hell
- 50+ additional cards
- Advanced AI behaviors
- Boss-specific mechanics
- Save/Load system
- Card crafting system
- Achievement framework
- Advanced visual effects
- Audio integration helpers

---

## Tutorial Series Structure

### Episode 1: "Summoning the Framework" (25 minutes)
- Plugin installation and setup
- Creating first basic cards
- Understanding the card data structure
- Setting up a simple battle scene

### Episode 2: "Binding Your First Contracts" (30 minutes)
- Implementing turn-based combat
- Player vs AI battle system
- Card playing mechanics
- Basic win/lose conditions

### Episode 3: "Collecting Infernal Souls" (35 minutes)
- Post-battle reward system
- Deck building mechanics
- Card collection management
- Progression between battles

### Episode 4: "Descending Through Hell's Layers" (30 minutes)
- Layer progression system
- Difficulty scaling
- Boss encounter framework
- Player advancement

### Episode 5: "Polishing Your Infernal Creation" (40 minutes)
- UI/UX improvements
- Visual effects integration
- Audio system hookups
- Final game feel polish

---

## Marketing Hooks for Keves

### Character Integration
- **"Today I'll teach you mortals how to create your own hellish card game!"**
- **"These soul contracts will bind players to your game forever!"**
- **"I've spent eons perfecting these infernal mechanics!"**
- **"Even Lucifer himself would be impressed by this boss battle system!"**

### Tutorial Theming
- Present code as "ancient demonic knowledge"
- Frame debugging as "exorcising bugs from your creation"
- Describe optimization as "making your game run smoother than silk in the underworld"
- Reference each feature as part of Keves' grand plan for digital domination

---

## Success Metrics

### Free Plugin Goals
- 1000+ downloads in first month
- 75%+ positive ratings on UE Marketplace
- Strong tutorial series engagement (10k+ views per episode)

### Premium Plugin Goals  
- 10%+ conversion rate from free to paid
- $2000+ revenue in first quarter
- Community-created content using the framework

### Long-term Vision
- Establish Keves as the go-to character for game development tutorials
- Build email list of developers interested in future plugins
- Create sustainable income stream supporting your family during your health challenges

---

## Development Timeline

### Week 1-2: Core Framework
- C++ card system implementation
- Basic Blueprint node creation
- Simple battle mechanics

### Week 3-4: Content Creation
- First 3 hell layers
- Boss encounters
- Card balancing

### Week 5-6: Polish & Tutorial Prep
- UI integration
- Tutorial scene setup
- Documentation writing

### Week 7: Launch Preparation
- Marketplace submission
- Tutorial video recording
- Marketing material creation

---

*"Remember, mortals - every great empire starts with a single contract. Let's make yours legendary!"* - Keves