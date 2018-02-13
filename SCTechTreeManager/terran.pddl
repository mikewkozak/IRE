(define (domain starcraft)
  (:requirements :strips :typing)
  (:types unit building upgrade)
  (:action barracks
     :parameters (?ut - unit ?bd - building ?up - upgrade)
     :precondition (exists command_center)
     :effect (exists barracks)
	 )
)