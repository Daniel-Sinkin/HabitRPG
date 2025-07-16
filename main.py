"""
Further ideas:
    - Bounty system, at start of day randomly select out of the pool of active grind spots
        and the first 2 grind sessions there are rolled 2x and give special pool specific currency
    - Randomly infect Obsidian Notes with monsters, work on notes to clear them, gives you
        rewards
    - Set up Projects, get incremental rewards for milestones
    - Implement Crafting System
    - Implement a Gacha Type System
    - Track stats like # obsidian notes, # consecutive days worked
"""

import random
from dataclasses import dataclass, field
from abc import ABC, abstractmethod
from typing import Any
import time


@dataclass
class QuestStage:
    lore: str
    completed: bool = False


@dataclass
class Quest:
    id: str
    name: str
    description: str
    stages: list[QuestStage]
    current_stage: int = 0
    completed: bool = False
    lore_intro: str = ""
    lore_completion: str = ""

    def start(self) -> None:
        print(f"📜 Quest Started: {self.name}")
        print(f"🗺️ {self.description}")
        print(f"✨ {self.lore_intro}\n")

    def advance(self) -> None:
        if self.completed:
            print(f"✅ Quest already completed: {self.name}")
            return

        if self.current_stage >= len(self.stages):
            self.completed = True
            print(f"\n🏁 Quest Complete: {self.name}")
            print(f"🎉 {self.lore_completion}")
            return

        stage = self.stages[self.current_stage]
        stage.completed = True
        print(f"🔎 Stage {self.current_stage + 1}: {stage.lore}")
        self.current_stage += 1

        if self.current_stage >= len(self.stages):
            self.completed = True
            print(f"\n🏁 Quest Complete: {self.name}")
            print(f"🎉 {self.lore_completion}")


@dataclass(frozen=True)
class Item:
    name: str
    descr: str
    effect: str | None = None  # Later we will have actual effect hooks


items: dict[str, dict[str, Item]] = {
    "common": {
        "lockbox": Item(
            name="Lockbox (Common)", descr="Open to get an common Item", effect=""
        ),
        "worn_training_sword": Item(
            name="Worn Training Sword",
            descr="A dented but dependable blade, once used by squired-in-training",
            effect="(1/3)% Double XP gain on grind, 2 charges then breaks",
        ),
        "gatherers_satchel": Item(
            name="Gatherer's Satchel",
            descr="A threadbare pouch with room for a few herbs, logs, or copper ore.",
            effect="10% Chance to double resource gain",
        ),
    },
    "uncommon": {
        "lockbox": Item(
            name="Lockbox (Uncommon)", descr="Open to get an uncommon Item", effect=""
        ),
        "card_deck_gambler": Item(
            name="Card Deck (Gambler's Potential)",
            descr="Who can risk more than he who has nothing to lose?",
            effect="Gamble this card, do 4 grind sessions to complete gamble, 2/3 chance to upgrade, 1/3 chance to get destroyed",
        ),
        "basic_recipe": Item(
            name="Basic Recipe",
            descr="A basic crafting recipe.",
            effect="Learn a basic crafting recipe from one of the crafting schools. Gain 20 XP.",
        ),
        "mirror_of_echoes": Item(
            name="Mirror of Echoes",
            descr="A handheld mirror that hums softly. It remembers your effort.",
            effect="On next repeated task, gain +50% XP if done within 24h of original.",
        ),
        "charred_map_fragment": Item(
            name="Charred Map Fragment",
            descr="A scorched scrap of parchment. The edges whisper of forgotten paths.",
            effect="Unlocks a hidden side quest chain after completing 3 exploration-type tasks.",
        ),
        "dusty_journal_page": Item(
            name="Dusty Journal Page (Echos from the Past)",
            descr="Covered in half-legible scrawlings, but one entry stands out.",
            effect="Select any task you've neglected for over a week. Completing it now grants double rewards and logs a 'Redemption' milestone.",
        ),
        "clockwork_beetle": Item(
            name="Clockwork Beetle",
            descr="This tiny mechanical creature clicks softly, keeping time with your focus.",
            effect="If a focus task is completed in a single uninterrupted sprint, gain double Gold.",
        ),
        "half_full_flask": Item(
            name="Half-Full Flask",
            descr="You're not sure what's in it, but it smells like courage.",
            effect="Once per week, use to skip a daily without penalty.",
        ),
        "whispering_locket": Item(
            name="Whispering Locket",
            descr="An old, tarnished locket that seems to hum when you're alone. It won't open... yet.",
            effect="Adds a mysterious entry to your collection log. Triggers a 3-task mini-quest once examined.",
        ),
    },
    "rare": {
        "lockbox": Item(name="Lockbox (Rare)", descr="", effect=""),
        "chronomancers_sandglass": Item(
            name="Chronomancer's Sandglass",
            descr="This cracked hourglass was once used to stretch seconds into focus-filled eternities.",
            effect="Do a special grind that lasts 45 minutes, has significantly higher Rare dropchance.",
        ),
    },
    "epic": {
        "lockbox": Item(
            name="Lockbox (Epic)",
            descr="Who knowns why mystical wonders this contains, need to bringt it to an arcane locksmith.",
            effect="Can only be opened by an arcane locksmith.",
        )
    },
}


class Reward(ABC):
    @abstractmethod
    def award(self) -> None:
        """Apply the reward (currently just prints)."""
        pass


class RewardNothing(Reward):
    def award(self) -> None:
        print("Go nothing 😥")


class RewardCurrency(Reward):
    def __init__(self, xp: int, gold: int):
        self.xp = xp
        self.gold = gold

    def award(self) -> None:
        print(f"🏅 +{self.xp} XP, +{self.gold} Gold")


class RewardResource(Reward):
    def __init__(self, resource_type: str, amount: int):
        self.resource_type = resource_type
        self.amount = amount

    def award(self) -> None:
        print(f"🪓 +{self.amount} {self.resource_type}")


class RewardItem(Reward):
    def __init__(self, tier: str | None = None, pet: str | None = None):
        self.tier = tier
        self.pet = pet

    def award(self) -> None:
        if self.pet:
            print(f"🐾 Found a pet: {self.pet}!")
        else:
            assert self.tier is not None
            print(f"🎁 Found a {self.tier.capitalize()} item!")


class Rewards(ABC):
    @abstractmethod
    def pull(self, n: int) -> list[Any]:
        """Pull `n` rewards from the table"""
        pass


class GrindRewards(Rewards):
    def __init__(self):
        self.rewards = {
            "nothing": 0.25,
            "resource": 0.5,
            "common": 0.25,
            "uncommon": 0.125,
            "rare": 0.0625,
            "epic": 0.03125,
            "legendary": 0.015625,
            "pet": 0.015625,
        }

        self.reward_types = list(self.rewards.keys())
        self.probs = [self.rewards[k] for k in self.reward_types]

    def pull(self, n: int) -> list[Reward]:
        reward_list: list[Reward] = []

        for _ in range(n):
            # Always get some XP + Gold
            xp = 10
            gold = random.randint(1, 5)
            reward_list.append(RewardCurrency(xp, gold))

            drop = random.choices(self.reward_types, weights=self.probs, k=1)[0]

            if drop == "nothing":
                reward_list.append(RewardNothing())
                continue  # No bonus
            elif drop == "resource":
                t1 = random.choice(["wood_oak", "iron", "marrentill"])
                t0 = random.choice(["wood", "copper", "guam_leaf"])
                if random.random() < 0.5:
                    amt = random.randint(3, 6)
                    reward_list.append(RewardResource(t0, amt))
                else:
                    amt = random.randint(1, 2)
                    reward_list.append(RewardResource(t1, amt))
            elif drop in {"common", "uncommon", "rare", "epic", "legendary"}:
                reward_list.append(RewardItem(tier=drop))
            elif drop == "pet":
                reward_list.append(RewardItem(pet="??Mystery Pet??"))
            else:
                raise AssertionError("No reward in GrindReward")

        assert len(reward_list) == 2 * n
        return reward_list


player_info = {
    "player_stats": {"xp": 200},
    "currency": {
        "gold": 59,
        "woodworking": {"wood": 0, "wood_oak": 2},
        "mining": {"copper": 0, "iron": 4},
        "herblore": {"guam_leaf": 3, "marrentill": 2},
    },
    "items": [
        items["uncommon"]["charred_map_fragment"],
        items["uncommon"]["dusty_journal_page"],
        items["uncommon"]["card_deck_gambler"],
        items["epic"]["lockbox"],
        items["uncommon"]["lockbox"],
        items["uncommon"]["lockbox"],
        items["rare"]["lockbox"],
        items["epic"]["lockbox"],
        items["common"]["lockbox"],
    ],
    "progress_stats": {
        "grinds_completed": 20,
        "quests_completed": 0,
    },
    "active_quests": {
        "forgotten_path": Quest(
            id="forgotten_path",
            name="The Forgotten Path",
            description="Follow the trail hinted at by the Charred Map Fragment.",
            lore_intro="The parchment crackles in your hands. Faint ink reveals a trail long lost to time...",
            lore_completion="",
            stages=[
                QuestStage(lore=""),
                QuestStage(lore=""),
                QuestStage(lore=""),
            ],
        )
    },
}


def pull_item(tier: str) -> Item:
    if tier not in items:
        raise ValueError(f"No items available for tier: {tier}")

    item_pool = items[tier]
    item = random.choice(list(item_pool.values()))

    # Titan forge logic: uncommon has a 10% chance to become a rare
    if tier == "uncommon" and random.random() < 0.10:
        print(f"💥 Titan Forged! {item.name} has been upgraded to Rare!")
        rare_pool = items["rare"]
        return random.choice(list(rare_pool.values()))

    return item


n_completed = 4
total_completed = int(player_info["progress_stats"]["grinds_completed"]) + n_completed
print(
    f"You have completed {n_completed}, so now you have {total_completed} total completions ({0.5 * total_completed:.1f} hours)"
)
for reward in GrindRewards().pull(n_completed):
    print("Pulling Grind Rewards")
    time.sleep(2.0 + random.random() * 3.0)
    reward.award()
    time.sleep(random.random() * 3.0)
    print()

if False:
    print(pull_item("uncommon").name)

if False:
    player_info["active_quests"]["forgotten_path"].advance()
