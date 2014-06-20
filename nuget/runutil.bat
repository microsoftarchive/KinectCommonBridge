@echo off
.\updatenugetversion\Release\updatenugetversion %1
del %1
ren %12 %1
