import React, { useState, useEffect, useCallback } from 'react';
import PropTypes from 'prop-types';

import CssBaseline from '@mui/material/CssBaseline';
import { ThemeProvider, StyledEngineProvider, createTheme } from '@mui/material/styles';
import makeStyles from '@mui/styles/makeStyles';
import AppBar from '@mui/material/AppBar';
import Toolbar from '@mui/material/Toolbar';
import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import IconButton from '@mui/material/IconButton';
import Badge from '@mui/material/Badge';
import Tooltip from '@mui/material/Tooltip';

import CloseIcon from "@mui/icons-material/Close";
import NotificationsIcon from "@mui/icons-material/Notifications";
import SyncAltIcon from "@mui/icons-material/SyncAlt";
import SyncProblemIcon from "@mui/icons-material/SyncProblem";

import { green } from '@mui/material/colors';

import AlertDialog from './Components/AlertDialog';
import SubjectSelect from './Components/SubjectSelect';
import SubjectDetail from './Components/SubjectDetail';

//import update from 'immutability-helper';

import './App.css';

import useInterval from './useInterval';
import { fetchJsonWithTimeout } from './rest.js';


let theme = createTheme();
theme = createTheme(theme, {
    root: {
      flexGrow: 1,
      backgroundColor: theme.palette.background.paper,
    },
    title: {
      flexGrow: 1,
      display: 'none',
      //[t.breakpoints.up('sm')]: {
      //  display: 'block',
      //},
    },
    tabbar: {
      flexGrow: 1,
      //color: theme.palette.primary.dark,
      //background: theme.palette.background.paper,
    },
});

function TabPanel(props) {
  const { children, value, index, ...other } = props;

  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`simple-tabpanel-${index}`}
      aria-labelledby={`simple-tab-${index}`}
      {...other}
    >
      {value === index && (
        <Box p={3}>
          {children}
        </Box>
      )}
    </div>
  );
}

TabPanel.propTypes = {
  children: PropTypes.node,
  index: PropTypes.any.isRequired,
  value: PropTypes.any.isRequired,
};

function a11yProps(index) {
  return {
    id: `simple-tab-${index}`,
    key: index,
    'aria-controls': `simple-tabpanel-${index}`,
  };
}

function App() {
  const [domain, setDomain] = useState("undef");
  //const [subjectkeys, setSubjectkeys] = useState([]);             // simple list
  const [opensubjectkeys, setOpensubjectkeys] = useState([]);     // simple list
  const [opentabindex, setOpentabindex] = useState(null);         // integer index into simple array, or null when no tab selected
  const [subjects, setSubjects] = useState([]);                   // array of subject objects indexed by subject key; only open subjects listed in opensubjectkeys will be fully populated
  const [alerts, setAlerts] = useState([]);                       // array of alert objects
  const [closedalerts, setClosedalerts] = useState([]);           // simple  list of alerts that have been closed
  const [seq, setSeq] = useState(-1);                             // integer
  const [alertseq, setAlertseq] = useState(-1);                   // integer
  const [casecounts, setCasecounts] = useState([]);               // array of integers indexed by subject key
  const [connected, setConnected] = useState(false);              // boolean
  const [autocompletevalue, setAutocompletevalue] = useState(''); // string in autocomplete input field
  const [forceupdate, setForceupdate] = useState(false);          // boolean used to force a subject update when opensubjects/tabs change
  const [alertDialogOpen, setAlertDialogOpen] = useState(false);  // boolean

  const useStyles = makeStyles((t) => ({
    root: {
      flexGrow: 1,
      backgroundColor: theme.palette.background.paper,
    },
    title: {
      flexGrow: 1,
      display: 'none',
      [t.breakpoints.up('sm')]: {
        display: 'block',
      },
    },
    tabbar: {
      flexGrow: 1,
      color: theme.palette.primary.dark,
      background: theme.palette.background.paper,
    },
  }));
  
  //const classes = useStyles();

  // given a list of subject objects, build a new casecounts array and update
  // casecount (state) accordingly.
  const updateCasecounts = useCallback((newsubjects) => {
    let ccs = [];
    let updatecasecounts = false;

    newsubjects.forEach((s) => {
      ccs[s.key] = s.casekeys.length;
      if (!(casecounts.includes(s.key)) || (casecounts[s.key] !== s.casekeys.length)) {
        updatecasecounts = true;
      }
    });
    if (updatecasecounts) {
      setCasecounts(ccs);
      console.log("casecounts updated!");
    }
  }, [casecounts, setCasecounts]);

  // Once on startup load initial values
  useEffect( () => {
    console.log('useEffect');
    fetchJsonWithTimeout("/domain")
    .then((json) => {
      setDomain(json.label);
      //setSubjectkeys(json.subjectkeys);
      let sbjs = [];
      if ('subjects' in json) {
        // instead of using somewhat random indexing, re-index it by key value for much easier use!
        for (let i = 0; i < json.subjects.length; i++) {
          let s = json.subjects[i];
          if (typeof(s) === 'object') {
            sbjs[s.key] = s;
          }
        }
      }
      setSubjects(sbjs);
      //OLD: setSubjects(json.subjects.sort((a, b) => (a.key > b.key) ? 1 : -1));
      updateCasecounts(json.subjects);
      //setAlertseq(json.alertseq); // this prevents alert badge from showing at startup
      setSeq(json.seq);
      setConnected(true);
    })
    .catch(error => console.log(error));
  }, [setDomain, setSubjects, setAlertseq, setSeq, setConnected] ); // setSubjectkeys

  // Load data from REST API. Attempt to be smart and only pull needed data
  const pollData = () => {
    //console.log("poll data");
    // First load any alerts...
    fetchJsonWithTimeout("/alerts")
    .then((json) => {
      //console.log("- conn: %o, seq: %d, alertseq: %d", connected, seq, alertseq);
      if (connected !== true) {  // update only when necessary to minimize flashing
        setConnected(true);
      }
      if (alertseq !== json.alertseq || alerts.length !== json.alerts.length) {
        //console.log("Updating alerts because seq=%d/%d (%d alerts)", json.alertseq, alertseq, json.alerts.length);
        setAlertseq(json.alertseq);
        setAlerts(json.alerts);
      }
      if (seq !== json.seq || forceupdate) {
        console.log('pollData: seq=%d forceupdate=%o', seq, forceupdate);
        setSeq(json.seq);
        if (domain !== "undef") {
          // This block updates other things when we know the sequence id has changed
          // First, update subject data. Note that we are NOT doing a deep compare here - we know something changed because of seq
          fetchJsonWithTimeout("/subjects?details=" + opensubjectkeys.toString())
          .then((json) => {
            let sbjs = [];
            if ('subjects' in json) {
              // instead of using somewhat random indexing, re-index it by key value for much easier use!
              for (let i = 0; i < json.subjects.length; i++) {
                let s = json.subjects[i];
                if (typeof(s) === 'object') {
                  sbjs[s.key] = s;
                }
              }
            }
            setSubjects(sbjs);
            updateCasecounts(sbjs);
            if (forceupdate) {
              setForceupdate(false);
            }
          })
          .catch(error => console.log(error));
        }
      }
    })
    .catch(error => {
      console.log(error);
      setConnected(false);
    });
  };

  // Set up regular polling by time interval to check for updates
  useInterval(pollData, 1000);

  // Return an array of alert ids that have not been closed
  const openAlerts = useCallback(() => {
    return alerts.filter(a => !closedalerts.includes(a.id));
  }, [alerts, closedalerts]);

  // User selected a different tab to open
  const handleTabChange = (event, newValue) => {
    setOpentabindex(newValue);
    setForceupdate(true);
    pollData();
  };

  // When user selects a new subject to add as a tab, wire it up.
  const handleAddSubjectTab = (event, newValue, reason) => {
    if ((newValue === null) || (!subjects) || (!(newValue instanceof Object)) || (!('name' in newValue))) {
      return;
    }
    console.log("handleAddSubjectTab: %d", newValue.key);
    let subjtab = subjects.find(s => {
      if (s && ('name' in s)) {
        return(s.name === newValue.name);
      }
      return false;
    });
    if (subjtab) {
      let k = subjtab.key;
      if (!(opensubjectkeys.includes(k))) {
        setOpentabindex(opensubjectkeys.length);  // 0-indexed
        setOpensubjectkeys(opensubjectkeys.concat(k)); // returns a new array like we need
        setAutocompletevalue("");
        setForceupdate(true);
        pollData();
      }
    }
  };

  // handle tab close buttons
  const handleTabClose = useCallback((event, value) => {
    // close the tab panel referenced by subject id
    event.stopPropagation();
    let i = opensubjectkeys.indexOf(value); // first get the index of the tab being closed, or -1 if not found
    // then figure out how to update the open tab index
    if (i >= 0) { // if found...
      let oldcount = opensubjectkeys.length;  // remember the original count
      if (oldcount > 1) { // or >0 next render after filter below
        if (opentabindex > 0) {
          setOpentabindex(opentabindex - 1); // keep same tab by decrementing index
        } else {
          // 1st tab removed, cannot decrement, so stay 0 to get the next tab -- do nothing
        }
      } else {
        // presumably no more tabs left
        setOpentabindex(-1);
      }
      setOpensubjectkeys(opensubjectkeys.filter(k => k !== value)); // then remove it on next render
      setForceupdate(true);
      pollData();
    }
  }, [opensubjectkeys, setOpentabindex, opentabindex, setOpensubjectkeys, setForceupdate, pollData]);

  // Open the alert dialog when user clicks the alerts button.
  const handleAlertButtonClick = (e) => {
    setAlertDialogOpen(!alertDialogOpen);
  };

  ///////////////////////////////RENDER////////////////////////////////////////
  console.log('App: rendering');
  let openalerts = openAlerts();
  let openalertcount = openalerts.length;

  return (
    <StyledEngineProvider injectFirst>
      <ThemeProvider theme={theme}>
        <CssBaseline />
        <div className="classes.root">
          <AppBar position="static">
            <Toolbar>
              <Typography className="classes.title" variant="h6" noWrap>
                EA FDD Console for: {domain}
              </Typography>
              <SubjectSelect
                subjects={subjects}
                casecounts={casecounts}
                opensubjectkeys={opensubjectkeys}
                autocompletevalue={autocompletevalue}
                setAutocompletevalue={setAutocompletevalue}
                handleAddSubjectTab={handleAddSubjectTab}
              />
              &nbsp;
              <IconButton
                style={{ color: "white" }}
                aria-controls="alert-dialog"
                aria-haspopup="dialog"
                onClick={handleAlertButtonClick}
                size="large">
                <Badge
                  badgeContent={openalertcount > 9 ? "9+" : openalertcount}
                  showZero={false}
                  color="error"
                >
                  <NotificationsIcon />
                </Badge>
              </IconButton>
              <AlertDialog id="alert-dialog" alerts={openalerts} closedalerts={closedalerts} setClosedalerts={setClosedalerts} alertDialogOpen={alertDialogOpen} setAlertDialogOpen={setAlertDialogOpen}/>
              &nbsp;
              {
                connected ?
                  <Tooltip title="Connected"><SyncAltIcon aria-label="connected" style={{ color: green[500] }}/></Tooltip>
                :
                  <Tooltip title="No connection"><SyncProblemIcon aria-label="unable to connect" color="error" fontSize="large"/></Tooltip>
              }
            </Toolbar>
          </AppBar>
        </div>
        <div className="classes.tabbar">
          <AppBar position="static" className="classes.tabbar">
            <Tabs
              value={opentabindex}
              variant="scrollable"
              scrollButtons="auto"
              indicatorColor="secondary"
              textColor="white"
              onChange={handleTabChange}
              aria-label="open subjects"
            >
              {
                opensubjectkeys.map((sid,idx) => {
                  return (
                      ((sid in subjects) && ('name' in subjects[sid]))
                    ?
                      <Tab
                        label=<span>{subjects[sid].name+" "}&nbsp;&nbsp;<Badge badgeContent={casecounts[sid]} color="error"/>&nbsp;&nbsp;<IconButton component="div" size="small" onClick={event => handleTabClose(event, sid)}><CloseIcon /></IconButton></span>
                        {...a11yProps(idx)}
                      />
                    :
                      ''
                  );
                })
              }
            </Tabs>
          </AppBar>
        </div>
        <div className="classes.root">
          {
            opensubjectkeys.length > 0 ?
              opensubjectkeys.map((sid,idx) =>
                <TabPanel value={opentabindex} index={idx} key={idx}>
                  {
                    (subjects && subjects[sid] && 'features' in subjects[sid] && 'cases' in subjects[sid]) && 'rulekits' in subjects[sid] ?
                      <SubjectDetail subject={subjects[sid]} />
                    :
                      <Typography>(loading...)</Typography>
                  }
                </TabPanel>
              )
              :
              <TabPanel value="none" index="none" key={999}>
                <Typography>Select subjects to surveil from the selection box above</Typography>
              </TabPanel>
          }
          <p>
            {seq}
          </p>
        </div>
      </ThemeProvider>
    </StyledEngineProvider>
  );
}

export default App;
