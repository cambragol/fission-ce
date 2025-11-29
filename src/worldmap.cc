    wmGenData.currentCarAreaId = area;
}

void wmForceEncounter(int map, unsigned int flags)
{
    if ((wmForceEncounterFlags & (1 << 31)) != 0) {
        return;
    }

    wmForceEncounterMapId = map;
    wmForceEncounterFlags = flags;

    // I don't quite understand the reason why locking needs one more flag.
    if ((wmForceEncounterFlags & ENCOUNTER_FLAG_LOCK) != 0) {
        wmForceEncounterFlags |= (1 << 31);
    } else {
        wmForceEncounterFlags &= ~(1 << 31);
    }
}

} // namespace fallout