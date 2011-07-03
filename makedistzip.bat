@rem va dans le drive du script
%~d0
@rem va dans le répetoire du script
cd "%~dp0"

set ZIP="C:\Program Files\7-Zip\7z.exe"

@rem -up0q0r2x1y2z1w2
@rem update switches :
@rem ---actions---
@rem 0 Ignore file (don't create item in new archive for this file)
@rem 1 Copy file (copy from old archive to new)
@rem 2 Compress (compress file from disk to new archive)
@rem 3 Create Anti-item (item that will delete file or directory during extracting). This feature is supported only in 7z format.
@rem ---states---
@rem -p File exists in archive, but is not matched with wildcard. -> ignore (don't create item in new archive for this file)
@rem -q File exists in archive, but doesn't exist on disk -> ignore
@rem -r File doesn't exist in archive, but exists on disk. -> compress (compress file from disk to new archive)
@rem -x File in archive is newer than the file on disk. -> copy (copy from old archive to new)
@rem -y File in archive is older than the file on disk. -> compress
@rem -z File in archive is same as the file on disk -> copy
@rem -w Can not be detected what file is newer (times are the same, sizes are different) -> Compress
%ZIP% u -up0q0r2x1y2z1w2 -tzip -xr!.svn ../pbniregex.zip @distfiles.lst
@pause
