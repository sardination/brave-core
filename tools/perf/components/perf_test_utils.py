#!/usr/bin/env python
# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.
from __future__ import annotations

import os
import subprocess
import json
import sys
import logging
import shlex
import shutil
import time

from copy import deepcopy

# pylint: disable=no-name-in-module,import-error
from components import path_util, browser_binary_fetcher, perf_profile
from components.perf_config import PerfConfiguration
with path_util.SysPath(path_util.PYJSON5_DIR):
  import json5
# pylint: enable=no-name-in-module,import-error


def GetRevisionNumberAndHash(revision: str) -> tuple[str, str]:
  """Returns pair [revision_number, sha1]. revision_number is a number "primary"
  commits from the begging to `revision`.
  Use this to get the commit from a revision number:
  git rev-list --topo-order --first-parent --reverse origin/master
  | head -n <rev_num> | tail -n 1 | git log -n 1 --stdin
  """

  brave_dir = os.path.join(path_util.SRC_DIR, 'brave')
  subprocess.check_call(['git', 'fetch', 'origin', revision],
                        cwd=brave_dir,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
  git_hash = subprocess.check_output(['git', 'rev-parse', 'FETCH_HEAD'],
                                     cwd=brave_dir).rstrip()
  rev_number_args = [
      'git', 'rev-list', '--topo-order', '--first-parent', '--count',
      'FETCH_HEAD'
  ]
  logging.debug('Run binary: %s', ' '.join(rev_number_args))
  rev_number = subprocess.check_output(rev_number_args, cwd=brave_dir).rstrip()
  return (rev_number.decode('utf-8'), git_hash.decode('utf-8'))


def RunSingleTest(binary,
                  config,
                  out_dir,
                  profile_dir,
                  is_ref,
                  verbose,
                  extra_browser_args,
                  extra_benchmark_args,
                  is_local_run,
                  local_run_label=''):
  args = [sys.executable]
  if not is_local_run:
    args.append(
        os.path.join(path_util.SRC_DIR, 'testing', 'scripts',
                     'run_performance_tests.py'))
  args.append(os.path.join(path_util.SRC_DIR,
              'tools', 'perf', 'run_benchmark'))

  benchmark = config['benchmark']

  logs: list[str] = []
  if is_local_run:
    assert (local_run_label is not None)
    args.append(benchmark)

    args.append('--results-label=' + local_run_label)
    args.append(f'--output-dir={out_dir}')
  else:
    if is_ref:
      benchmark += '.reference'
    args.append(f'--benchmarks={benchmark}')
    args.append('--isolated-script-test-output=' +
                os.path.join(out_dir, config['benchmark'], 'output.json'))

  if profile_dir:
    args.append(f'--profile-dir={profile_dir}')
  args.append('--browser=exact')
  args.append(f'--browser-executable={binary}')
  args.append('--pageset-repeat=%d' % config['pageset-repeat'])
  if 'stories' in config:
    for story in config['stories']:
      args.append(f'--story={story}')

  # TODO_perf: should be is_chromium, see _GetVariationsBrowserArgs
  if not is_ref:
    extra_browser_args.append('--use-brave-field-trial-config')

  args.extend(extra_benchmark_args)

  if verbose:
    args.append('--show-stdout')

  args.append('--extra-browser-args=' + shlex.join(extra_browser_args))

  logging.debug('Run binary: %s', ' '.join(args))

  try:
    output = subprocess.check_output(args,
                                     stderr=subprocess.STDOUT,
                                     cwd=os.path.join(path_util.SRC_DIR,
                                                      'tools', 'perf'))
    logging.debug(output)
    return True, logs
  except subprocess.CalledProcessError as e:
    logging.error(e.output.decode())
    return False, []


def ReportToDashboard(is_ref, configuration_name, revision, output_dir):
  args = [
      path_util.VPYTHON_3_PATH,
      os.path.join(path_util.SRC_DIR, 'tools', 'perf',
                   'process_perf_results.py')
  ]
  args.append(f'--configuration-name={configuration_name}')
  args.append(f'--task-output-dir={output_dir}')
  args.append('--output-json=' + os.path.join(output_dir, 'results.json'))

  revision_number, git_hash = GetRevisionNumberAndHash(revision)
  logging.debug('Got revision %s git_hash %s', revision_number, git_hash)

  build_properties = {}
  build_properties['bot_id'] = 'test_bot'
  build_properties['builder_group'] = 'brave.perf'

  build_properties['parent_builder_group'] = 'chromium.linux'
  build_properties['parent_buildername'] = 'Linux Builder'
  build_properties['recipe'] = 'chromium'
  build_properties['slavename'] = 'test_bot'

  # keep in sync with _MakeBuildStatusUrl() to make correct build urls.
  build_properties['buildername'] = ('chrome'
                                     if is_ref else 'brave') + '/' + revision
  build_properties['buildnumber'] = '001'

  build_properties[
      'got_revision_cp'] = 'refs/heads/main@{#%s}' % revision_number
  build_properties['got_v8_revision'] = revision_number
  build_properties['got_webrtc_revision'] = revision_number
  build_properties['git_revision'] = git_hash
  build_properties_serialized = json.dumps(build_properties)
  args.append('--build-properties=' + build_properties_serialized)

  logging.debug('Run binary: %s', ' '.join(args))
  try:
    output = subprocess.check_output(args, stderr=subprocess.STDOUT)
    logging.debug(output)
    return True, [], revision_number
  except subprocess.CalledProcessError as e:
    logging.error(e.output.decode())
    return False, ['Reporting ' + revision + ' failed'], None


def GetConfigPath(config_path):
  if os.path.isfile(config_path):
    return config_path

  absolute_config = os.path.join(path_util.BRAVE_PERF_DIR, 'configs',
                                 config_path)
  if os.path.isfile(absolute_config):
    return absolute_config
  raise RuntimeError(f'Bad config {config_path}')


def LoadConfig(config: str) -> dict:
  config_path = GetConfigPath(config)
  with open(config_path, 'r') as config_file:
    return json5.load(config_file)


class CommonOptions:
  do_run_test = True
  do_report = False
  report_on_failure = False
  local_run = False
  working_directory = ''
  tests_config = None

  @classmethod
  def make_local(cls, working_directory, tests_config):
    options = CommonOptions()
    options.working_directory = working_directory
    options.tests_config = tests_config
    options.local_run = True
    return options

  @classmethod
  def from_args(cls, args, tests_config):
    options = CommonOptions()
    options.do_run_test = not args.report_only
    options.do_report = not args.no_report and not args.local_run
    options.report_on_failure = args.report_on_failure
    options.local_run = args.local_run
    options.working_directory = args.working_directory
    options.tests_config = tests_config
    return options


class RunableConfiguration:
  config: PerfConfiguration
  binary_path: str
  out_dir: str

  status_line: str = ''
  logs: list[str] = []
  profile_dir: str

  def __init__(self, config: dict, binary_path: str, out_dir: str):
    self.config = config
    self.binary_path = binary_path
    self.out_dir = out_dir

  def PrepareProfile(self, common_options: CommonOptions):
    start_time = time.time()
    self.profile_dir = perf_profile.GetProfilePath(
        self.config.profile, self.binary_path, common_options.working_directory)
    self.status_line += f'Prepare {(time.time() - start_time):.2f}s '

  def RunTests(self, common_options: CommonOptions) -> bool:
    has_failure = False

    self.PrepareProfile(common_options)

    start_time = time.time()
    for test_config in common_options.tests_config:
      benchmark = test_config['benchmark']
      if common_options.local_run:
        test_out_dir = os.path.join(self.out_dir, os.pardir, benchmark)
      else:
        test_out_dir = os.path.join(self.out_dir, 'results')
      logging.info('Running test %s', benchmark)

      test_success, test_logs = RunSingleTest(
          self.binary_path, test_config, test_out_dir, self.profile_dir,
          self.config.chromium, True, self.config.extra_browser_args,
          self.config.extra_benchmark_args, common_options.local_run,
          self.config.label)
      self.logs.extend(test_logs)

      if not test_success:
        has_failure = True
        error = f'Test case {benchmark} failed on binary {self.binary_path}'
        error += '\nLogs: ' + os.path.join(test_out_dir, benchmark, benchmark,
                                           'benchmark_log.txt')
        self.logs.append(error)

    spent_time = time.time() - start_time
    self.status_line += f'Run {spent_time:.2f}s '
    self.status_line += 'FAILURE  ' if has_failure else 'OK  '
    return not has_failure

  def ReportToDashboard(self) -> bool:
    logging.info('Reporting to dashboard %s...', self.config.label)
    start_time = time.time()
    assert (self.config.dashboard_bot_name is not None)
    report_success, report_failed_logs, revision_number = ReportToDashboard(
        self.config.chromium, self.config.dashboard_bot_name,
        'refs/tags/' + self.config.tag, os.path.join(self.out_dir, 'results'))
    spent_time = time.time() - start_time
    self.status_line += f'Report {spent_time:.2f}s '
    self.status_line += 'OK, ' if report_success else 'FAILURE, '
    self.status_line += f'Revnum: #{revision_number}'
    if not report_success:
      self.logs.extend(report_failed_logs)
    return report_success

  def ClearArtifacts(self, common_options: CommonOptions):
    if common_options.local_run:
      for test_config in common_options.tests_config:
        benchmark = test_config['benchmark']
        artifacts_dir = os.path.join(self.out_dir, os.pardir, benchmark,
                                     'artifacts')
        shutil.rmtree(artifacts_dir)

  def Run(self, common_options: CommonOptions) -> bool:
    logging.info('##Label: %s binary %s', self.config.label, self.binary_path)
    run_tests_ok = True
    report_ok = True

    if common_options.do_run_test:
      run_tests_ok = self.RunTests(common_options)
    if common_options.do_report:
      if run_tests_ok or common_options.report_on_failure:
        report_ok = self.ReportToDashboard()
    self.logs.append(self.status_line)
    if run_tests_ok and report_ok and not self.config.save_artifacts:
      self.ClearArtifacts(common_options)

    return run_tests_ok and report_ok, self.logs


def PrepareBinariesAndDirectories(
        configurations: list[PerfConfiguration],
        common_options: CommonOptions) -> list[RunableConfiguration]:
  runable_configurations: list[RunableConfiguration] = []
  for config in configurations:
    if config.tag == config.label:
      description = config.tag
    else:
      description = f'{config.label}[tag_{config.tag}]'
    out_dir = os.path.join(common_options.working_directory, description)
    binary_path = None

    if common_options.do_run_test:
      shutil.rmtree(out_dir, True)
      binary_path = browser_binary_fetcher.PrepareBinary(
          out_dir, config.tag, config.location, config.chromium)
      logging.info('%s : %s directory %s', description, binary_path, out_dir)
    runable_configurations.append(
        RunableConfiguration(config, binary_path, out_dir))
  return runable_configurations


def SpawnConfigurationsFromTargetList(
    target_list: list[str],
    base_configuration: PerfConfiguration) -> \
        list[PerfConfiguration]:
  configurations: list[PerfConfiguration] = []
  for target_string in target_list:
    config = deepcopy(base_configuration)
    config.tag, config.location = browser_binary_fetcher.ParseTarget(
        target_string)
    #TODO_perf: add more early validation?
    if not config.tag:
      raise RuntimeError(f'Can get the tag from target {target_string}')
    config.label = config.tag
    configurations.append(config)
  return configurations


def ParseConfigurations(
        configurations_list: list[dict]) -> list[PerfConfiguration]:
  configurations: list[PerfConfiguration] = []
  for serialized_config in configurations_list:
    config = PerfConfiguration(serialized_config)
    #TODO_perf: add more early validation?
    if config.tag:
      if not config.label:
        config.label = config.tag
    elif config.label:
      config.tag = config.label  # TODO_perf: do we need this?
    else:
      raise RuntimeError(
          f'label or tag should be specified {serialized_config}')
    configurations.append(config)
  return configurations


def RunConfigurations(configurations: list[PerfConfiguration],
                      common_options: CommonOptions) -> bool:
  runable_configurations = PrepareBinariesAndDirectories(
      configurations, common_options)

  has_failure = False
  logs: list[str] = []
  for config in runable_configurations:
    result, config_logs = config.Run(common_options)
    if not result:
      has_failure = True
    logs.extend(config_logs)

  if common_options.local_run:
    for test_config in common_options.tests_config:
      benchmark = test_config['benchmark']
      logs.append(benchmark + ' : file://' + os.path.join(
          common_options.working_directory, benchmark, 'results.html'))

  if logs != []:
    logging.info('Logs:')
    for item in logs:
      logging.info(item)

  if has_failure:
    logging.error('Summary: not ok')
  else:
    logging.info('Summary: OK')
