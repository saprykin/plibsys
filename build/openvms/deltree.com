$     ivfy = f$verify(0)
$     on control_y then goto out
$!
$! DELTREE.COM - AUTHORS: Joseph G. Slater
$!                        Alexander Saprykin
$!
$! DELTREE deletes a subdirectory tree. You must set your default to 
$! the parent directory of the subdirectory tree you wish to delete. 
$! For instance, if you have a tree [SLATER.MAIN] and you want to 
$! delete the [.MAIN] subtree, you must set your default to [SLATER]
$! Assuming you have the symbol DELTREE set up, you would type:
$!
$!   deltree main
$! 
$ orig_dir = f$enviroment("DEFAULT")
$ error_free = 1
$
$ CONTEXT0 = 0
$ CONTEXT1 = 1
$ CONTEXT2 = 2
$ CONTEXT3 = 3
$ CONTEXT4 = 4
$ CONTEXT5 = 5
$ CONTEXT6 = 6
$ CONTEXT7 = 7
$ CONTEXT8 = 8
$ CONTEXT9 = 9
$! 
$ if "''p1'" .eqs. "" then goto out
$!
$ level = 1
$ md = p1 + ".dir"
$ md = f$search(md)                            ! Look for First level directory
$!
$ if md .eqs. "" then goto out
$!
$ set prot=w:d 'p1'.dir
$ set default [.'p1']                          ! Set to first level directory
$!
$ srch:
$     df = f$search("*.dir", CONTEXT'level')   ! Any directories?
$     if "''df'" .eqs. "" then goto delthem
$     df = "''f$parse(df,,,"NAME")'"           ! Get the directory name
$     set prot=w:d 'df'.dir                    ! Let it be deleted
$     set default [.'df']                      ! Go down a level
$     level = level + 1
$     if level .gt. 8 then goto out
$     goto srch
$!
$ delthem:
$     del *.*;*                                ! Delete all files
$!    if .not. $STATUS then error_free = 0
$     set default [-]
$     level = level - 1                        ! Go Back up a level
$     if level .eq. 0 then goto finish
$     goto srch
$!
$ finish:
$     if .not. error_free then goto out
$     del 'p1'.dir;*
$     goto out
$!
$ out:
$     set default 'orig_dir'
$     if .not. ivfy then set noverify
