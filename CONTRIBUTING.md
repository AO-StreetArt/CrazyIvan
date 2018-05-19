We love pull requests from everyone.  By participating in this project, you agree to abide by the Code of Conduct, specified in the CODE_OF_CONDUCT.md file.

Start by forking and cloning the repository.  The easiest way to handle dependencies
is by using the Vagrant file.  Install Vagrant (https://www.vagrantup.com/), then,
from the main directory of the repo:

`vagrant up`

`vagrant ssh`

`scl enable devtoolset-7 bash`

`cd ../../vagrant`

Ensure that the tests pass:

`make && make test`

Vagrant creates a Virtual Machine hosting the code and dependencies, and shares the
folder containing the code between your machine and the VM.  This means that you can
make your updates on your computer in your favorite IDE, and build within the VM.  
Once you've made you changes and written tests for your changes, then ensure that all tests pass:

`make clean && make && make test`

Push to your fork and submit a pull request.

At this point you're waiting on us. We like to at least comment on pull requests within three business days (and, typically, one business day). We may suggest some changes or improvements or alternatives.

Some things that will increase the chance that your pull request is accepted:

* Write tests
* Comment your code
* Write a good commit message
* Use [CppLint](https://github.com/google/styleguide/tree/gh-pages/cpplint) and follow the [Google C++ Style Guide](https://github.com/google/styleguide) wherever possible
