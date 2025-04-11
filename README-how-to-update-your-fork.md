# How to integrate updates from upstream/master to main

First, commit any local changes.  Then:
```
git checkout main
git pull upstream main
<...resolve any merge conflicts...>
git push [origin main]
```
