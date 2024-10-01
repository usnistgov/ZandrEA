// useInterval as implemented here:
// https://blog.bitsrc.io/polling-in-react-using-the-useinterval-custom-hook-e2bcefda4197

import { useEffect, useRef } from 'react';

const useInterval = (callback, delay) => {
  const savedCallback = useRef();
  // Remember the latest callback
  useEffect( () => {
    savedCallback.current = callback;
  }, [callback]);

  // Set up interval
  useEffect( () => {
    function tick() {
      savedCallback.current();
    }
    if (delay !== null) {
      const id = setInterval(tick, delay);
      return () => { clearInterval(id); };
    }
  }, [callback, delay]);
};

export default useInterval;
