import React from 'react';

class Clock extends React.Component {
   constructor(props) {
      super(props);
      this.state = {
         time: new Date().toString()
      };
   }
   componentDidMount() {
      this.intervalID = setInterval(
         () => this.tick(),
         1000
      );
   }
   componentWillUnmount() {
      clearInterval(this.intervalID);
   }
   tick() {
      this.setState({
         time: new Date().toString()
      });
   }

   render() {
      return (
         <p className="App-clock">
         The time is {this.state.time}.
         </p>
      );
   }
}

export default Clock;
