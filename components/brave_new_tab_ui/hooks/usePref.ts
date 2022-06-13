import { addChangeListener, getPreferences, sendSavePref } from "../api/preferences";
import { PrefHookManager, usePref } from "./PrefHookManager";

const prefManager = new PrefHookManager(getPreferences, sendSavePref, addChangeListener);

// Similar API to useState, but for getting/setting NewTab preferences.
export const useNewTabPref = <T extends keyof NewTab.Preferences>(pref: T) => usePref(prefManager, pref);
