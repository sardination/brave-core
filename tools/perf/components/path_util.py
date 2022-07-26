# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.

import contextlib
import os
import sys

SRC_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, os.pardir,
                 os.pardir))

BRAVE_SRC_DIR = os.path.join(SRC_DIR, 'brave')

BRAVE_PERF_DIR = os.path.join(SRC_DIR, 'brave', 'tools', 'perf')

BRAVE_PERF_PROFILE_DIR = os.path.join(BRAVE_PERF_DIR, 'profiles')

BRAVE_PERF_BUCKET = 'brave-telemetry'

BRAVE_DEPOT_TOOLS_DIR = os.path.join(BRAVE_SRC_DIR, 'vendor', 'depot_tools')

PYJSON5_DIR = os.path.join(SRC_DIR, 'third_party', 'pyjson5', 'src')

GOOGLE_AUTH_DIR = os.path.join(SRC_DIR, 'third_party', 'catapult',
                               'third_party', 'google-auth')

VPYTHON_3_PATH = os.path.join(
    BRAVE_DEPOT_TOOLS_DIR,
    'vpython3.bat' if sys.platform == 'win32' else 'vpython3')


@contextlib.contextmanager
def SysPath(path, position=None):
  if position is None:
    sys.path.append(path)
  else:
    sys.path.insert(position, path)
  try:
    yield
  finally:
    if sys.path[-1] == path:
      sys.path.pop()
    else:
      sys.path.remove(path)

def GetBinaryPath(browser_dir):
  if sys.platform == 'win32':
    return os.path.join(browser_dir, 'brave.exe')
  if sys.platform == 'darwin':
    binary_dir = os.path.join(browser_dir, 'Contents', 'MacOS')
    for file in os.listdir(binary_dir):
      if file.startswith('Brave Browser'):
        return os.path.join(binary_dir, file)
    raise RuntimeError(f'Couldn\'t find a binary in {binary_dir}')

  raise RuntimeError(f'Unsupported platfrom {sys.platform}')
