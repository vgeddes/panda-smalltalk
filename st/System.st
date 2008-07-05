
"accessing"

System method!
at: key put: object
	^ globals at: key put: object!

System method!
at: key
	^ globals at: key!


"system control"

System method!
startupSystem
	self exitWithResult: nil doIt!

System method!
exitWithResult: anObject
	<primitive: 'UndefinedObject_exitWithResult'>
	self primitiveFailed!

System method!
exit
	self exitWithResult: nil!