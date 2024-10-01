# How to integrate updates from upstream/master to master

First, commit any local changes.  Then:
```
git checkout master
git pull upstream master
<...resolve any merge conflicts...>
git push [origin master]
```
