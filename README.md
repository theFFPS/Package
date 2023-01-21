# Package
## Overview
**Package** is a Package Manager wrote in C++. Can be used on every linux distribution.
* [Requirements](https://github.com/theFFPS/APIServerPP#requirements)
* [Building](https://github.com/theFFPS/APIServerPP#building)
* [Installation](https://github.com/theFFPS/APIServerPP#installation)
* [Documentation](https://github.com/theFFPS/APIServerPP#documentation)
> * [Repositiories](https://github.com/theFFPS/APIServerPP#repositories)
> * [Packages](https://github.com/theFFPS/APIServerPP#packages)
## Requirements
* [CURL](https://github.com/curl/curl)
* [CURL++](https://github.com/jpbarrette/curlpp)
* [Make](https://git.savannah.gnu.org/cgit/make.git)
* [G++ (GCC)](https://github.com/gcc-mirror/gcc)
## Building
```c++
make
```
## Installation
```c++
sudo make install
sh scripts/PostBuild.sh
```
## Documentation
### Repositories
#### Create new
```c++
package repo new
```
#### Insert package
```c++
package repo insert-pkg <PKG:in current dir>
```
#### Remove package
```c++
package repo remove-pkg <PKG:in current dir>
```
#### Print repositories list
```c++
package repo list
```
#### Add repository
```c++
package repo add <REPOS:multiple:url>
```
#### Remove repository
```c++
package repo remove <REPOS:multiple:url>
```
#### Overwrite repositories with default
```c++
package repo default
```
### Packages
#### Install packages
```c++
package add <PKGS:multiple:id/filename:if local/name>
```
#### Remove packages
```c++
package remove <PKGS:multiple:id>
```
#### List packages
```c++
package list-packages installes/available:soon
```
#### Create package
* Setup metadata
> 
```c++
package project <DIR> set <FIELD> <VALUE>
```
> If needed to get field 
```c++
package project <DIR> get <FIELD>
```
> If needed to get all fields 
```c++
package project <DIR> get-all
```
> If needed to get available fields 
```c++
package project <DIR> available-fields
```
* Create directory packageData in <DIR> with project
> * Build project and create data.tgz (tar archive) in packageData
> * Fetch checksum (sha256) of packageData/data.tgz
* Execute 
```c++
package package .
```
