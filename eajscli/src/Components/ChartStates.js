import { useState } from 'react';
import PropTypes from "prop-types";
import 'react-responsive-modal/styles.css';
import { Modal } from 'react-responsive-modal';
import {
  VictoryChart, VictoryAxis, VictoryScatter, VictoryTheme, VictoryTooltip, VictoryLegend, Border,
} from 'victory';

import Histogram from "./Histogram";

const bumpz = { overlay: { zindex: 999990 } };

const ChartStates = (props) => {
  const { pane, name, width, data, valueFields, argumentField, stateField, fills, symbol, traceHistograms } = props;
  //console.log("Charting state data %o", data);

  const [modalOpen, setModalOpen] = useState(null);

  function onCloseModal() {
    setModalOpen(null);
  };

  if (data === null || data.length < 1) {
    return null;
  }

  const bartop = 1.0;
  const totalbarheight = (bartop * valueFields.length) + bartop;
  const domain = { x: [data[0][argumentField], data[data.length-1][argumentField]], y: [0, totalbarheight] };

  //const myWidth = 512;
  const myHeight = 100 + (15 * valueFields.length);
  const myLegendWidth = 100;
  const leftOffset = 100;

  let histopanel = null;
  if (modalOpen !== null) {
    // modalOpen is the index into the list of valueField tags (?)
    // find the right histogram object
    //console.log("ChartStates: adding histogram of %s: %o", modalOpen, traceHistograms);
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
        data={data}
        domainPadding={{ x: [20, 20], y: [0, 0] }}
        domain={domain}
        theme={VictoryTheme.material}
        height={myHeight}
        width={width}
        padding={{ left: leftOffset, top: 20, right: myLegendWidth + 10, bottom: 30 }}
        scale={{x: 'time'}}
      >
        <VictoryAxis
          dependentAxis
          key={99}
          //label={name}
          orientation="right"
          offsetX={myLegendWidth + 10}
          style={{
            //grid: { stroke: "none" },
            ticks: { stroke: "none" },
            tickLabels: { fontSize: 11 }, // angle: -33
          }}
          tickCount={valueFields.length}
          tickValues={valueFields}
          events={[{
            target: "tickLabels",
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
        {
          data.map((d,i) => {
            return (
              <VictoryAxis
                dependentAxis
                key={`p${pane}a${i}`}
                //label={d[stateField]}
                axisValue={d[argumentField]}
                tickValues={[]}
                style={{
                  axis: { stroke: "rgba(200, 200, 200, 0.5)" },
                  tickLabels: { fill: "none" },
                  ticks: { stroke: "none" },
                  grid: { stroke: "none" },
                }}
              />
            );
          })
        }
        {
          valueFields.map((vf,i) => {
            return (
              <VictoryScatter
                key={`p${pane}r${i}`}
                name={vf}
                style={{
                  data: {
                    fill: ({ datum }) => fills[datum[vf][stateField]].fill,
                    stroke: ({ datum }) => fills[datum[vf][stateField]].stroke,
                    fillOpacity: ({ datum }) => fills[datum[vf][stateField]].opacity,
                    strokeWidth: 2,
                  },
                }}
                data={data}
                scale="linear"
                x={argumentField}
                y={(d) => ((d[vf].b + 1) * d[vf].y)}
                size={6}
                symbol={symbol}
                labelComponent={<VictoryTooltip/>}
                labels={({ datum }) => `${vf}: ${datum[vf][stateField]}`}
              />
            );
          })
        }
        <VictoryAxis />
        <VictoryLegend
          x={20} y={30} gutter={10} width={leftOffset - 40}
          borderComponent={<Border width={leftOffset - 40}/>}
          padding={5}
          style={{
            border: { stroke: "black", },
            //title: { fontSize: 18 },
          }}
          title={name}
          centerTitle
          data={[]}
        />
      </VictoryChart>
      {histopanel}
    </div>
  );
};

ChartStates.propTypes = {
  pane: PropTypes.number.isRequired,
  name: PropTypes.string.isRequired,
  width: PropTypes.number.isRequired,
  data: PropTypes.arrayOf(PropTypes.object).isRequired,
  valueFields: PropTypes.arrayOf(PropTypes.string).isRequired,
  argumentField: PropTypes.string.isRequired,
  stateField: PropTypes.string.isRequired,
  fills: PropTypes.object.isRequired,
  symbol: PropTypes.string.isRequired,
  traceHistograms: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default ChartStates;
