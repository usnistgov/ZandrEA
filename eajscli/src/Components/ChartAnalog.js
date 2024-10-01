import { useState } from 'react';
import PropTypes from "prop-types";
import 'react-responsive-modal/styles.css';
import { Modal } from 'react-responsive-modal';
import {
  VictoryChart, VictoryLine, VictoryScatter, VictoryAxis, VictoryTheme, VictoryLegend, VictoryVoronoiContainer, Border,
} from 'victory';

import Histogram from './Histogram';

const fills = ["red", "green", "blue", "orange", "cyan", "violet", "yellow", "black" ];

const bumpz = { overlay: { zindex: 999990 } };

const ChartAnalog = (props) => {
  const { pane, width, data, valueFields, argumentField, xmin, xmax, ymin, ymax, yunits, traceHistograms } = props;
  //console.log("Charting vf=%o data=%o", valueFields, data);

  const [modalOpen, setModalOpen] = useState(null);

  function onCloseModal() {
    setModalOpen(null);
  };

  const lines = valueFields.map((vf,i) =>
    <VictoryLine
      key={`p${pane}l${i}`}
      name={vf}
      y={`${vf}.y`}
      x={argumentField}
      interpolation="linear"
      data={data}
      style={{
        data: { stroke: fills[i] },
        labels: { fill: fills[i] },
        parent: { border: "1px solid #ccc" },
      }}
    />
  );

  let missing = [];
  let invalid = [];

  for (let i = 0; i < data.length; i++) {
    if (! data[i].v) {  // If this timestamp has invalid data...
      let xmissing = {};
      let xinvalid = {};
      for (const t of valueFields) {
        if (data[i][t].u) {           // unavailable
          xmissing.x = data[i].x;
          xmissing[t] = { y: data[i][t].y };
        }
        if (data[i][t].i) {           // invalid
          xinvalid.x = data[i].x;
          xinvalid[t] = { y: data[i][t].y };
        }
      }
      if ('x' in xmissing) {
        missing.push(xmissing);
        console.log("Chart: missing point i=%d : %o", i, xmissing);
      }
      if ('x' in xinvalid) {
        invalid.push(xinvalid);
        console.log("Chart: invalid point i=%d: %o", i, xinvalid);
      }
    }
  }
  if (invalid.length > 0) {
    console.log("Chart: invalid data: %o", invalid);
  }
  if (missing.length > 0) {
    console.log("Chart: missing data: %o", missing);
  }

  let voronoiBlacklist = [];

  const missingPoints = valueFields.map((vf, i) => {
    const myvf = vf.replace(/^[^:]*:/, '');
    const name = `MIS-p${pane}-${myvf}`;
    voronoiBlacklist.push(name);
    return (
      <VictoryScatter
        key={`p${pane}m${i}`}
        name={name}
        y={`${vf}.y`}
        x={argumentField}
        interpolation="linear"
        data={missing}
        style={{
          data: { fill: fills[i] },
          labels: { fill: fills[i] },
        }}
        symbol="plus"
        size={10}
      />
    );
  });
  const invalidPoints = valueFields.map((vf, i) => {
    const myvf = vf.replace(/^[^:]*:/, '');
    const name = `INV-p${pane}-${myvf}`;
    voronoiBlacklist.push(name);
    return (
      <VictoryScatter
        key={`p${pane}i${i}`}
        name={name}
        y={`${vf}.y`}
        x={argumentField}
        interpolation="linear"
        data={invalid}
        style={{
          data: { fill: fills[i] },
          labels: { fill: fills[i] },
        }}
        symbol="circle"
        size={7}
      />
    );
  });

  //const myWidth = 512;
  const myHeight = 150;
  const myLegendWidth = 100;
  const leftOffset = 100;
  const maxdp = 4;
  const mindiff = Math.pow(10, 0-maxdp);

  function dp(x) {
    const factor = Math.pow(10, maxdp);
    let x1 = Math.trunc(x * factor);    // x = 8.0250, x1 = 80250
    if (x1 % 10) {
      return 4;
    }
    if (x1 % 100) {
      return 3;
    }
    if (x1 % 1000) {
      return 2;
    }
    if (x1 % 10000) {
      return 1;
    }
    return 0;
  };

  // compute an expanded min/max range if it's smaller than the desired precision
  function adj(min, max, precision) {
    const range = max - min;
    if (range < mindiff) {
      const c = Math.pow(10, 0-precision);
      return([min-(3*c), min+(3*c)]);
    }
    return([min,max]);
  };

  const yprecision = Math.max(dp(ymin), dp(ymax));  // how many decimal places to use?
  const domain = {x: [xmin, xmax], y: adj(ymin, ymax, yprecision)};

  let histopanel = null;
  if (modalOpen !== null) {
    // modalOpen is the index into the list of valueField tags (?)
    // find the right histogram object
    //console.log("ChartAnalog: adding histogram of %s: %o", modalOpen, traceHistograms);
    const histogram = traceHistograms[modalOpen];
    if (histogram) {
      histopanel =
        <Modal open={true} onClose={onCloseModal} center closeOnEsc closeOnOverlayClick styles={bumpz} >
          <h3>{histogram.idtext}</h3>
          <Histogram
            key={histogram.key}
            id={histogram.key}
            type={histogram.type}
            idtext={histogram.idtext}
            bartype={histogram.bartype}
            barcount={histogram.barcount}
            caption={histogram.caption}
            barheights={histogram.bar_heights}
            barlabels={histogram.bar_labels}
            leftbarvalue={histogram.left_bar_value}
            eachbarincr={histogram.each_bar_incr}
            rightbarvalue={histogram.right_bar_value}
            modeindex={histogram.mode}
            spanindex={histogram.span}
            modeoptions={histogram.mode_options}
            spanoptions={histogram.span_options}
            knobs={histogram.knobs}
          />
        </Modal>;
    }
  }

  return (
    <div>
      <VictoryChart
        domain={domain}
        domainPadding={20}
        data={data}
        theme={VictoryTheme.material}
        height={myHeight}
        width={width}
        padding={{ left: leftOffset, top: 20, right: myLegendWidth + 10, bottom: 30 }}
        containerComponent={
          <VictoryVoronoiContainer
            //voronoiDimension="x"
            voronoiBlacklist={voronoiBlacklist}
            labels={({ datum }) => `${datum._x.toLocaleDateString()} ${datum._x.toLocaleTimeString()}: ${Number.isFinite(datum._y) ? datum._y.toFixed(yprecision) : datum._y.toString()}`}
            preserveAspectRatio="none"
            responsive={true}
          />
        }
      >
        <VictoryLegend
          x={20} y={30} gutter={10} width={leftOffset - 20}
          borderComponent={<Border width={leftOffset - 20}/>}
          padding={5}
          orientation="vertical"
          style={{ border: { stroke: "black", }, }}
          data={ valueFields.map((vf,i) => { return { name: vf, symbol: { fill: fills[i] } }; } ) }
          events={[{
            target: "labels",
            eventHandlers: {
              onClick: (ev, objprops) => {
                //console.log("Pane legend clicked with objprops=%o", objprops);
                // objprops.datum.name == objprops.text == trace tag
                // objprops.index == (index of trace tag)
                // need to figure out how to identify the parent trace and hook in its modal histograms here
                setModalOpen(objprops.text);
                return [];
              }
            }
          }]}
        />
        <VictoryAxis />
        <VictoryAxis
          dependentAxis
          label={yunits}
          orientation="right"
          offsetX={myLegendWidth + 10}
          style={{
            tickLabels: { angle: -33 },
            axisLabel: { padding: 60, angle: -90 },
          }}
          tickFormat={(t) => `${t.toFixed(dp(t))}`}
        />
        {lines}
        {invalidPoints}
        {missingPoints}
      </VictoryChart>
      {histopanel}
    </div>
  );
};

ChartAnalog.propTypes = {
  pane: PropTypes.number.isRequired,
  width: PropTypes.number.isRequired,
  data: PropTypes.arrayOf(PropTypes.object).isRequired,
  valueFields: PropTypes.arrayOf(PropTypes.string).isRequired,
  argumentField: PropTypes.string.isRequired,
  xmin: PropTypes.object.isRequired,
  xmax: PropTypes.object.isRequired,
  ymin: PropTypes.number.isRequired,
  ymax: PropTypes.number.isRequired,
  yunits: PropTypes.string.isRequired,
  traceHistograms: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default ChartAnalog;
