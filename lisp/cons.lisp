(define x (cons 1 2))
(define z (cons x 3))

(car z)

(define (gcd a b)
		(if (= b 0)
			a
			(gcd b (remainder a b))))

(define (reduce x)
		(let ((low (gcd (car x) (cdr x))))
		(cons (/ (car x) low) (/ (cdr x) low))))

;; Reduce fraction to its lowest term
(reduce (cons 25 40))