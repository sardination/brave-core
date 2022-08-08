// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

const fs = require('fs')
const program = require('commander')
const path = require('path')
const config = require('../lib/config')
const util = require('../lib/util')
const Log = require('../lib/sync/logging')
const chalk = require('chalk')

program
  .version(process.env.npm_package_version)
  .option('--gclient_file <file>', 'gclient config file location')
  .option('--gclient_verbose', 'verbose output for gclient')
  .option('--target_os <target_os>', 'target OS')
  .option('--target_arch <target_arch>', 'target architecture')
  .option('--target_android_base <target_android_base>', 'target Android OS level for apk or aab (classic, modern, mono)')
  .option('--init', 'initialize all dependencies')
  .option('--force', 'force reset all projects to origin/ref')
  .option('--src_sync [arg]', 'force sync or skip "src" dir sync (true/false/1/0)', JSON.parse)
  .option('--cleanup', 'remove obsolete gclient dirs')
  .option('--nohooks', 'Do not run hooks after updating')

const maybeInstallDepotTools = (options = config.defaultOptions) => {
  options.cwd = config.braveCoreDir

  if (!fs.existsSync(config.depotToolsDir)) {
    Log.progress('Install Depot Tools...')
    fs.mkdirSync(config.depotToolsDir)
    util.run('git', ['-C', config.depotToolsDir, 'clone', 'https://chromium.googlesource.com/chromium/tools/depot_tools.git', '.'], options)
    Log.progress('Done Depot Tools...')
  }

  const ninjaLogCfgPath = path.join(config.depotToolsDir, 'ninjalog.cfg');
  if (!fs.existsSync(ninjaLogCfgPath)) {
    // Create a ninja config to prevent (auto)ninja from calling goma_auth
    // each time. See for details:
    // https://chromium.googlesource.com/chromium/tools/depot_tools/+/main/ninjalog.README.md
    const ninjaLogCfgConfig = {
      'is-googler': false,
      'version': 3,
      'countdown': 10,
      'opt-in': false
    };
    fs.writeFileSync(ninjaLogCfgPath, JSON.stringify(ninjaLogCfgConfig))
  }
}

const buildRootGClientConfig = () => {
  let gclientConfig = {
    solutions: [
      {
        managed: '%False%',
        name: 'src',
        url: config.chromiumRepo,
        custom_deps: {
          'src/third_party/WebKit/LayoutTests': '%None%',
          'src/chrome_frame/tools/test/reference_build/chrome': '%None%',
          'src/chrome_frame/tools/test/reference_build/chrome_win': '%None%',
          'src/chrome/tools/test/reference_build/chrome': '%None%',
          'src/chrome/tools/test/reference_build/chrome_linux': '%None%',
          'src/chrome/tools/test/reference_build/chrome_mac': '%None%',
          'src/chrome/tools/test/reference_build/chrome_win': '%None%'
        },
        custom_vars: {
          'checkout_pgo_profiles': config.isBraveReleaseBuild() ? '%True%' :
                                                                  '%False%'
        }
      },
      {
        managed: '%False%',
        name: 'src/brave',
        // We do not use gclient to manage brave-core, so this should not
        // actually get used.
        url: 'https://github.com/brave/brave-core.git'
      }
    ]
  }
  if (process.env.GIT_CACHE_PATH) {
    config["cache_dir"] = process.env.GIT_CACHE_PATH
  }
  if (config.targetOS) {
    config['target_os'] = [config.targetOS]
  }

  let out = ''
  for (const [key, value] of Object.entries(gclientConfig)) {
    if (out.length) {
      out += '\n'
    }
    out += `${key} = ${JSON.stringify(value, null, 2)}`
  }
  out = out.replace(/"%(.*?)%"/gm, "$1")

  fs.writeFileSync(config.rootGClientFile, out)
}

const shouldUpdateChromium = (chromiumRef, latestFullySyncedRefFile) => {
  const headSHA = util.runGit(config.srcDir, ['rev-parse', 'HEAD'], true)
  const targetSHA = util.runGit(config.srcDir, ['rev-parse', chromiumRef], true)
  let lastestFullySyncedRef = undefined
  if (fs.existsSync(latestFullySyncedRefFile)) {
    lastestFullySyncedRef =
        fs.readFileSync(latestFullySyncedRefFile)
  }
  const needsUpdate = ((targetSHA !== headSHA) || (!headSHA && !targetSHA)) ||
      lastestFullySyncedRef != chromiumRef
  if (needsUpdate) {
    const currentRef = util.getGitReadableLocalRef(config.srcDir)
    console.log(
        `Chromium repo ${chalk.blue.bold('needs sync')}.\n  target is ${
            chalk.italic(chromiumRef)} at commit ${
            targetSHA || '[missing]'}\n  current commit is ${
            chalk.italic(currentRef || '[unknown]')} at commit ${
            chalk.inverse(
                headSHA || '[missing]')}\n  latest fully synced ref is: ${
                  lastestFullySyncedRef}`)
  } else {
    console.log(
        chalk.green.bold(`Chromium repo does not need sync as it is already ${
            chalk.italic(
                chromiumRef)} at commit ${targetSHA || '[missing]'}.`))
  }
  return needsUpdate
}

const chromiumSync = (program, options = {}) => {
  const requiredChromiumRef = config.getProjectRef('chrome')
  let args = [
    'sync', '--nohooks', '--reset', '--revision',
    'src@' + requiredChromiumRef, '--with_tags',
    '--with_branch_heads', '--upstream'
  ];

  const syncWithForce = program.init || program.force
  if (syncWithForce) {
    args = args.concat(['--force'])
  }
  if (program.cleanup) {
    if (util.isGitExclusionExists(config.srcDir, 'brave')) {
      args = args.concat(['-D'])
    } else {
      Log.warn('Cleanup is skipped, because brave dir is not ignored in src')
    }
  }

  const latestFullySyncedRefFile =
      path.join(config.rootDir, '.brave_latest_fully_synced_src_ref')
  const chromiumNeedsUpdate =
      shouldUpdateChromium(requiredChromiumRef, latestFullySyncedRefFile)
  const shouldSyncSrc = chromiumNeedsUpdate || syncWithForce || program.src_sync
  if (!shouldSyncSrc) {
    return false
  }

  if (program.src_sync !== undefined && !program.src_sync) {
    console.warn(chalk.yellow.bold('Chromium needed sync but received the flag to skip performing the update. Working directory may not compile correctly.'))
    return false
  }

  options.cwd = config.rootDir
  util.runGClient(args, options)
  util.addGitExclusion(config.srcDir, 'brave')
  fs.writeFileSync(latestFullySyncedRefFile, requiredChromiumRef)

  const postSyncChromiumRef = util.getGitReadableLocalRef(config.srcDir)
  Log.status(`Chromium is now at ${postSyncChromiumRef || '[unknown]'}`)
  return true
}

const braveSync = (program, options = {}) => {
  let args = ['sync', '--nohooks']
  const syncWithForce = program.init || program.force
  if (syncWithForce) {
    args = args.concat(['--force'])
  }
  if (program.cleanup) {
    args = args.concat(['-D'])
  }

  options.cwd = config.braveCoreDir
  util.runGClient(args, options)
}

async function RunCommand () {
  program.parse(process.argv)
  config.update(program)

  if (program.init || !fs.existsSync(config.depotToolsDir)) {
    maybeInstallDepotTools()
  }

  if (program.init || !fs.existsSync(config.rootGClientFile)) {
    buildRootGClientConfig()
  }

  Log.progress('Running gclient sync...')
  const didSyncChromium = chromiumSync(program)
  if (!didSyncChromium) {
    // If no Chromium sync was done, run sync inside `brave` to sync Brave DEPS.
    braveSync(program)
  }
  Log.progress('...gclient sync done')

  await util.applyPatches()

  if (!program.nohooks) {
    // Always run hooks from root, this will include Chromium and Brave hooks.
    // We don't check if they were successful or not, because runhooks step is
    // pretty quick in a no-op scenario.
    util.gclientRunhooks()
  }
}

Log.progress('Brave Browser Sync starting')
RunCommand()
.then(() => {
  Log.progress('Brave Browser Sync complete')
})
.catch((err) => {
  Log.error('Brave Browser Sync ERROR:')
  console.error(err)
  process.exit(1)
})
