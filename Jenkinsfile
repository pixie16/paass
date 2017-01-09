simpleBuild{
    env= [
        FOO : 42,
        BAR : "YASS"
    ]

    before_script = 'echo before'
    script = 'echo after $FOO'
    after_script = 'ls -lah'

    notifications = [
        email : "spaualus@utk.edu"
    ]
}