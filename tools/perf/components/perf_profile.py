# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.

import logging
import os
import sys
import subprocess
import hashlib
import uuid
import shutil
import shlex

from zipfile import ZipFile

from components import path_util  # pylint: disable=no-name-in-module

sys.path.append(os.path.join(path_util.SRC_DIR, 'third_party', 'depot_tools'))
import download_from_google_storage  # pylint: disable=import-error,wrong-import-position

# To upload a profile call:
# upload_to_google_storage.py some_profile.zip -b brave-telemetry
def DownloadFromGoogleStorage(sha1, output_path):
  gsutil = download_from_google_storage.Gsutil(
      download_from_google_storage.GSUTIL_DEFAULT_PATH)
  gs_path = 'gs://' + path_util.BRAVE_PERF_BUCKET + '/' + sha1
  logging.info('Download profile from %s to %s', gs_path, output_path)
  exit_code = gsutil.call('cp', gs_path, output_path)
  if exit_code:
    raise RuntimeError(f'Failed to download: {gs_path}')


def GetProfilePath(profile, binary, work_directory):
  if profile == 'clean':
    return None

  binary_path_hash = hashlib.sha1(binary.encode("utf-8")).hexdigest()[:6]
  profile_id = profile + '-' + binary_path_hash

  if not hasattr(GetProfilePath, 'profiles'):
    GetProfilePath.profiles = {}

  if profile in GetProfilePath.profiles:
    return GetProfilePath.profiles[profile_id]

  profile_dir = None
  if os.path.isdir(profile):  # local profile
    profile_dir = os.path.join(work_directory, 'profiles',
                               uuid.uuid4().hex.upper()[0:6])
    logging.debug('Copy %s to %s ', profile, profile_dir)
    shutil.copytree(profile, profile_dir)
    if not RebaseProfile(binary, profile_dir, []):
      raise RuntimeError(f'Failed to rebase {profile_dir}')
  else:
    zip_path = os.path.join(path_util.BRAVE_PERF_PROFILE_DIR, profile + '.zip')
    zip_path_sha1 = os.path.join(path_util.BRAVE_PERF_PROFILE_DIR,
                                 profile + '.zip.sha1')

    if not os.path.isfile(zip_path_sha1):
      raise RuntimeError(f'Unknown profile, file {zip_path_sha1} not found')

    sha1 = None
    with open(zip_path_sha1, 'r') as sha1_file:
      sha1 = sha1_file.read().rstrip()
    logging.debug('Expected hash %s for profile %s', sha1, profile)
    if not sha1:
      raise RuntimeError(f'Bad sha1 in {zip_path_sha1}')

    if not os.path.isfile(zip_path):
      DownloadFromGoogleStorage(sha1, zip_path)
    else:
      current_sha1 = download_from_google_storage.get_sha1(zip_path)
      if current_sha1 != sha1:
        logging.info(
            'Profile needs to be updated. Current hash %s, expected %s',
            current_sha1, sha1)
        DownloadFromGoogleStorage(sha1, zip_path)
    profile_dir = os.path.join(work_directory, 'profiles',
                               profile_id + '-' + sha1)

    if not os.path.isdir(profile_dir):
      os.makedirs(profile_dir)
      logging.info('Create temp profile dir %s for profile %s', profile_dir,
                   profile)
      zipfile = ZipFile(zip_path)
      zipfile.extractall(profile_dir)
      if not RebaseProfile(binary, profile_dir, []):
        raise RuntimeError(f'Failed to rebase {profile_dir}')

  logging.info('Use temp profile dir %s for profile %s', profile_dir, profile)
  GetProfilePath.profiles[profile_id] = profile_dir
  return profile_dir


def RebaseProfile(binary, profile_directory, extra_browser_args):
  logging.info('Rebasing dir %s using binary %s', profile_directory, binary)
  args = [
      sys.executable,
      os.path.join(path_util.SRC_DIR, 'tools', 'perf', 'run_benchmark'),
      'loading.desktop.brave'
  ]
  args.append('--story=BraveSearch_cold')
  args.append('--browser=exact')
  args.append(f'--browser-executable={binary}')
  args.append('--pageset-repeat=1')

  args.append(f'--profile-dir={profile_directory}')

  # See chromium desktop_browser_finder.py
  extra_browser_args.append('--update-source-profile')
  # TODO_perf: add  is_chromium, see _GetVariationsBrowserArgs
  extra_browser_args.append('--use-brave-field-trial-config')

  args.append('--extra-browser-args=' + shlex.join(extra_browser_args))

  args.append('--output-format=none')
  #TODO_perf: add output

  logging.debug('Run binary: %s', ' '.join(args))

  try:
    output = subprocess.check_output(args,
                                     stderr=subprocess.STDOUT,
                                     cwd=os.path.join(path_util.SRC_DIR,
                                                      'tools', 'perf'))
    logging.debug(output)
    return True
  except subprocess.CalledProcessError as e:
    logging.error(e.output.decode())
    return False
