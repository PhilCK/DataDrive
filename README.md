# Data Drive

This is a small utility library, based on somethings I read about decision/
condition tables.

The main principle is, execute a function when certain data exists or not.

The table below helps visualize how this works.
The function on the left  will be called if the data requirements are met.

_Contrived Table_

|Function       | User Data | Login Data | Network Data | Error Data |
|---------------|-----------|------------|--------------|------------|
| TryLogin()    | -         | Exists     | Exists       | -          |
| LoginFailed() | -         | -          | -            | Exists     |
| RetryLogin()  | Not Exist | Exists     | Exists       | Exists     |
| ShowLogout()  | Exists    | -          | Exists       | -          |
| ShowError()   | -         | -          | -            | Exists     |

You can Find an bigger example in the test folder. 

##  Build And Go

There is nothing special going on here, just build the 'C' files, with and
include the 'include' path.

`cc src/datadrive.c test/main.c -I ./include/ -Wall`

## Futher Reading

Things I read that made me want to write this.

- https://en.wikipedia.org/wiki/Decision_table
- https://en.wikipedia.org/wiki/Data-oriented_design
- https://www.dataorienteddesign.com/dodmain/node7.html
