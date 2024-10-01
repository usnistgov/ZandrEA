import axios from 'axios';

//
// CONFIGURE THESE IF NEEDED
//
// Default configuration is that the REST server is on the same host
// as the web service - if so, no change is needed.
//
const api_port_number = "9876";
const apibase = "http://" + window.location.hostname + ":" + api_port_number;
// 
// END OF CONFIGURATION
//

export const restInit = () => {
  return axios.create({baseURL: apibase, timeout: 1000});
}

export const restGetDomain = (conn) => {
  conn.get("/domain")
  .then(response => {
    console.log('restGetDomain');
    //const newState = update(this.state,
    //          {
    //	               domain: { $set: response.data.label },
    //	               subjectkeys: { $set: response.data.subjectkeys },
    //	               subjects: { $set: response.data.subjects },
    //	               alertseq: { $set: response.data.alertseq },
    //	               seq: { $set: response.data.seq },
    //	               connected: { $set: true },
    //	            }
    //       );
    //this.setState(newState);
  })
  .catch(error => console.log(error));
};

export const restGetUpdates = (conn) => {
  console.log('restGetUpdates');
};
