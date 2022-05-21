package ro.dissertation.dbapp.web.error;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;

@ControllerAdvice
public class GlobalExceptionHandler {

    private final Logger logger = LoggerFactory.getLogger(GlobalExceptionHandler.class);

    @ExceptionHandler({Exception.class})
    public ResponseEntity<String> handleException(Exception e) {
        logger.error("Exception encountered: {}", e.getMessage());

        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body("Bad request");
    }
}
