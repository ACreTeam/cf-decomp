# GitHub Actions

[The active Build workflow](../.github/workflows/build.yml) builds `RUUE01_00`
using `ghcr.io/acreteam/ac-build:main`, the same private build image used by
ac-decomp. The placeholder under `.github.example` is not an active workflow.

## Build Repository

The private [ACreTeam/ac-build](https://github.com/ACreTeam/ac-build) repository
contains the originals and publishes the private GHCR image. City Folk needs:

```text
orig/RUUE01_00/sys/main.dol
orig/RUUE01_00/files/rels/*.rel
```

The current revision has 187 RELs. The image contains the shared `/binutils`
and `/compilers` tools as well. After changing originals, wait for ac-build's
image-publishing workflow to succeed before rebuilding cf-decomp.

In the `ac-build` package's **Settings > Manage Actions access**, grant
`ACreTeam/cf-decomp` the **Read** role. Repository access to ac-build alone does
not grant the cf-decomp workflow permission to pull its package. See
[GitHub's package-access instructions](https://docs.github.com/en/packages/learn-github-packages/configuring-a-packages-access-control-and-visibility).

The workflow authenticates the container pull with `GITHUB_TOKEN` and
`packages: read`; no additional registry secret is needed once package access
is granted. Keep the build repository and image private.

## Workflow

Commit and push the source, headers, configuration and `.github/workflows/build.yml`
to cf-decomp. Pushes and in-repository pull requests run the build. The Actions
page also supports **Run workflow**. Fork pull requests skip the private-container
job; a maintainer can build a reviewed change on a branch in cf-decomp.

The workflow restores file timestamps with ac-decomp's `v2025.08` helper, copies
only this version's originals, caches generated build files, and runs:

```sh
python configure.py --map --version RUUE01_00 --binutils /binutils --compilers /compilers
ninja all_source progress build/RUUE01_00/report.json
```

The `progress` target depends on the DOL/REL SHA-1 checks. It also writes the
progress summary to the GitHub Actions job summary. A successful run uploads:

- `RUUE01_00_maps`: generated `.map`/`.MAP` files.
- `RUUE01_00_report`: `build/RUUE01_00/report.json` for decomp.dev.

For build notifications, watch `ACreTeam/cf-decomp` and enable **Actions** under
your [notification settings](https://github.com/settings/notifications). Choose
GitHub, email, or notifications only for failed workflows. This is a personal
account setting, not a workflow secret. See
[GitHub's notification instructions](https://docs.github.com/en/subscriptions-and-notifications/how-tos/managing-github-actions-notifications).

## decomp.dev

decomp.dev downloads objdiff report artifacts from GitHub Actions; the build does
not need a separate upload API key or a command that posts directly to the site.
The artifact and report filenames above follow its ingestion convention.

For the hosted site:

1. Make the **cf-decomp source repository** public when ready. Its current private
   visibility prevents normal hosted registration. Keep ac-build and its package
   private; the source workflow can still pull them using its package permission.
2. Push the workflow and project changes to `main`, then obtain a successful
   `Build` run containing the `RUUE01_00_report` artifact.
3. Sign in to [decomp.dev/manage/new](https://decomp.dev/manage/new) with a GitHub
   account that has admin access to `ACreTeam/cf-decomp`. Select the repository,
   enter the City Folk project details, select the `Build` workflow and save.
   Use the project's Refresh control if the initial report is not imported yet.
4. For automatic report updates and PR progress comments, install the
   [decomp-dev GitHub App](https://github.com/apps/decomp-dev) for cf-decomp and
   enable **PR comments** in the decomp.dev project settings.

decomp.dev marks a project hidden until it reaches **0.5% matched code**.
The current overall report is approximately **0.46%**, including the DOL and
all RELs, so registration/report ingestion can be set up before it appears in
the public listing. Configured game/SDK categories only cover the identified
units so far and are not whole-game percentages.

Hosted registration and visibility behavior are implemented in
[decomp.dev's management page](https://github.com/encounter/decomp.dev/blob/main/crates/web/src/handlers/manage.rs)
and [project visibility rule](https://github.com/encounter/decomp.dev/blob/main/crates/core/src/models.rs).
