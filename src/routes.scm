;; Routing Library in Scheme similar to Clojure's Compojure Library
;; Refer to https://github.com/weavejester/compojure
;;
;; summer 2017, peiker/ol
;;
;; usage illustration:
;;
;; (define-routes tst-routes
;;   (msg-begins-with "TOK1" (begin "got token1"))
;;   (msg-begins-with "TOK2" (begin "got token2")))
;;
;; (route my-routes "tok1") -> "got token 1"
;; (route my-routes "tok2") -> "got token 2"
;; (route my-routes "xy")   -> "xy"
;;


;; returns #t when string str begins with character sequence key
(define (string-starts-with str key)
  (let ((substr (substring str 0
                           (min (string-length key)
                                (string-length str)))))
    (string-ci=? substr key)))


;; Route string s trough list of lambdas (routes)
;; until it finds a matching one. Evaluate and return
;; matching expression in that case. Otherwise the string s
;; is returned.
;;
;; Example:
;;
;; (define my-routes
;;   (list
;;    (lambda (s) (if (string-starts-with s "TOK1") "got token 1" #f))
;;    (lambda (s) (if (string-starts-with s "TOK2") "got token 2" #f))
;;    ))
;;
;; (route my-routes "tok2")
;;
(define (route routes s)
  (let loop ((l routes))
    (if (pair? l)
        (let ((res ((car l) s)))
          (if res res (loop (cdr l))))
        s)))


;; Route macro definition which defines a route lambda expression
;; which evaluates body when message begins with token
;;
;; Example:
;;
;; (msg-begins-with "TOK1" (begin "got token1")) ->
;;    (lambda(s) (if (string-starts-with s "TOK1")
;;                   (begin "got token1")
;;                   #f))
;;
(define-macro (msg-begins-with tok . body)
  `(lambda (s)
     (if (string-starts-with s ,tok)
         (let ((args (substring s (string-length ,tok) (string-length s)))) . ,body)
         #f)))


;; Examples
;;
;; (define my-routes
;;   (list
;;    (msg-begins-with "TOK1" (begin "got token1"))
;;    (msg-begins-with "TOK2" (begin "got token2"))))
;;
;; (route my-routes "tok1")
;; (route my-routes "tok2")
;; (route my-routes "xy")



;; binds list of routes specified in body to symbol name 'defined-name'
;;
;; Example
;;
;; (define-routes tst-routes
;;   (msg-begins-with "TOKA" (begin "got token A"))
;;   (msg-begins-with "TOKB" (begin (string-append "got TOKB with args: " args))))
;;
;; (route tst-routes "TOKB3") -> "got TOKB with args: 3"
(define-macro (define-routes defined-name . body)
  `(define ,defined-name
     (list . ,body)))
