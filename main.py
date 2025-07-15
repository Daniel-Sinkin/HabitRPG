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
from dataclasses import dataclass
from abc import ABC, abstractmethod
from typing import Any
import time


@dataclass(frozen=True)
class Item:
    name: str
    descr: str
    effect: str | None = None  # Later we will have actual effect hooks


items: dict[str, list[Item]] = {
    "common": [
        Item(
            name="Worn Training Sword",
            descr="A dented but dependable blade, once used by squired-in-training",
            effect="(1/3)% Double XP gain on grind, 2 charges then breaks",
        ),
        Item(
            name="Gatherer's Satchel",
            descr="A threadbare pouch with room for a few herbs, logs, or copper ore.",
            effect="10% Chance to double resource gain",
        ),
    ],
    "uncommon": [
        Item(
            name="Card Deck (Gambler's Potential)",
            descr="Who can risk more than he who has nothing to lose?",
            effect="Gamble this card, do 4 grind sessions to complete gamble, 2/3 chance to upgrade, 1/3 chance to get destroyed",
        ),
        Item(
            name="Basic Recipe",
            descr="A basic crafting recipe.",
            effect="Learn a basic crafting recipe from one of the crafting schools. Gain 20 XP.",
        ),
    ],
    "rare": [
        Item(
            name="Chronomancer's Sandglass",
            descr="This cracked hourglass was once used to stretch seconds into focus-filled eternities.",
            effect="Do a special grind that lasts 45 minutes, has significantly higher Rare dropchance.",
        )
    ],
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
    "stats": {"xp": 50},
    "currency": {
        "gold": 19,
        "woodworking": {"wood": 0, "wood_oak": 2},
        "mining": {"copper": 0, "iron": 2},
        "herblore": {"guam_leaf": 3, "marrentill": 0},
    },
    "items": [],
}

for reward in GrindRewards().pull(5):
    print("Pulling Grind Rewards")
    time.sleep(2.0 + random.random() * 3.0)
    reward.award()
    time.sleep(random.random() * 3.0)
    print()
