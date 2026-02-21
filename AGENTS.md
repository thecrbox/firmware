# Source code

Use files in current dir and contents of `./src/` dir
Ignore `.git`, `.idea`; always exclude them
Go to `.esphome` only to check current upstream implementation; when you need to figure out the impl of a class there is no project source code there, only upstream code

# Workflow

When working on a feature:

- explain the idea and ask for confirmation
- when allowed, code, compile, code, compile, ...
- if needed, ask questions
- when done, report success and ask for test results if you can't do them on your own
- ask for test results
- if test ok, ask if you should create a commit; never commit anything before asking for confirmation

Do NOT manually create new components or features, if there are similar ones available on esphome official docs.

