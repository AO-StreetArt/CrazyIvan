We love pull requests from everyone.  By participating in this project, you agree to abide by the Code of Conduct, specified in the CODE_OF_CONDUCT.md file.

Keep in mind that we maintain a DebugDockerfile, which can be used to generate a development Docker image.  This will pre-install the application and dependencies for you, making
working on Crazy Ivan much easier.  Be sure to check out the [Developer Notes](http://crazyivan.readthedocs.io/en/latest/pages/dev_notes.html) for more information.

Start by forking and cloning the repository, then following the instructions to [build from source](http://crazyivan.readthedocs.io/en/latest/quickstart.html).

Ensure that the tests pass:

`make && make test`

Make your changes and write tests for your changes, then ensure that all tests pass:

`make clean && make && make test`

Push to your fork and submit a pull request.

At this point you're waiting on us. We like to at least comment on pull requests within three business days (and, typically, one business day). We may suggest some changes or improvements or alternatives.

Some things that will increase the chance that your pull request is accepted:

* Write tests
* Comment your code
* Write a good commit message
* Use [CppLint](https://github.com/google/styleguide/tree/gh-pages/cpplint) and follow the [Google C++ Style Guide](https://github.com/google/styleguide) wherever possible
