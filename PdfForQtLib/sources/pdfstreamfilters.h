//    Copyright (C) 2018-2019 Jakub Melka
//
//    This file is part of PdfForQt.
//
//    PdfForQt is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    PdfForQt is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDFForQt.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFSTREAMFILTERS_H
#define PDFSTREAMFILTERS_H

#include "pdfobject.h"

#include <QByteArray>

#include <memory>
#include <functional>

namespace pdf
{
class PDFStreamFilter;
class PDFSecurityHandler;

using PDFObjectFetcher = std::function<const PDFObject&(const PDFObject&)>;

/// Storage for stream filters. Can retrieve stream filters by name. Using singleton
/// design pattern. Use static methods to retrieve filters.
class PDFStreamFilterStorage
{
public:
    /// Retrieves filter by filter name. If filter with that name doesn't exist,
    /// then nullptr is returned. This function is thread safe.
    /// \param filterName Name of the filter to be retrieved.
    static const PDFStreamFilter* getFilter(const QByteArray& filterName);

    /// Returns decoded data from the stream
    /// \param stream Stream containing the data
    /// \param objectFetcher Function which retrieves objects (for example, reads objects from reference)
    /// \param securityHandler Security handler for Crypt filters
    static QByteArray getDecodedStream(const PDFStream* stream, const PDFObjectFetcher& objectFetcher, const PDFSecurityHandler* securityHandler);

    /// Returns decoded data from the stream, without object fetching
    /// \param stream Stream containing the data
    /// \param objectFetcher Function which retrieves objects (for example, reads objects from reference)
    /// \param securityHandler Security handler for Crypt filters
    static QByteArray getDecodedStream(const PDFStream* stream, const PDFSecurityHandler* securityHandler);

private:
    explicit PDFStreamFilterStorage();

    static const PDFStreamFilterStorage* getInstance();

    /// Maps names to the instances of the stream filters
    std::map<QByteArray, std::unique_ptr<PDFStreamFilter>> m_filters;

    /// Filter stream names can be specified in simplified (shorter) form.
    /// This map maps shorter form to the longer form.
    std::map<QByteArray, QByteArray> m_abbreviations;
};

class PDFStreamPredictor
{
public:
    /// Create predictor from stream parameters. If error occurs, exception is thrown.
    /// \param objectFetcher Function which retrieves objects (for example, reads objects from reference)
    /// \param parameters Parameters of the predictor (must be an dictionary)
    static PDFStreamPredictor createPredictor(const PDFObjectFetcher& objectFetcher, const PDFObject& parameters);

    /// Applies the predictor to the data.
    /// \param data Data to be decoded using predictor
    QByteArray apply(const QByteArray& data) const;

private:

    enum Predictor
    {
        NoPredictor = 1,
        TIFF = 2,
        PNG_None = 10,      ///< No prediction
        PNG_Sub = 11,       ///< Prediction based on previous byte
        PNG_Up  = 12,       ///< Prediction based on byte above
        PNG_Average = 13,   ///< Prediction based on average of previous nad current byte
        PNG_Paeth = 14,     ///< Nonlinear function
    };

    inline explicit PDFStreamPredictor() = default;

    inline explicit PDFStreamPredictor(Predictor predictor, int components, int bitsPerComponent, int columns) :
        m_predictor(predictor),
        m_components(components),
        m_bitsPerComponent(bitsPerComponent),
        m_columns(columns),
        m_stride(0)
    {
        m_stride = (m_columns * m_components * m_bitsPerComponent + 7) / 8;
    }

    /// Applies PNG predictor
    QByteArray applyPNGPredictor(const QByteArray& data) const;

    /// Applies TIFF predictor
    QByteArray applyTIFFPredictor(const QByteArray& data) const;

    Predictor m_predictor = NoPredictor;
    int m_components = 0;
    int m_bitsPerComponent = 0;
    int m_columns = 0;
    int m_stride = 0;
};

class PDFFORQTLIBSHARED_EXPORT PDFStreamFilter
{
public:
    explicit PDFStreamFilter() = default;
    virtual ~PDFStreamFilter() = default;

    /// Apply with object fetcher
    /// \param data Stream data to be decoded
    /// \param objectFetcher Function which retrieves objects (for example, reads objects from reference)
    /// \param parameters Stream parameters
    virtual QByteArray apply(const QByteArray& data, const PDFObjectFetcher& objectFetcher, const PDFObject& parameters, const PDFSecurityHandler* securityHandler) const = 0;

    /// Apply without object fetcher - it assumes no references exists in the streams dictionary
    /// \param data Stream data to be decoded
    /// \param parameters Stream parameters
    inline QByteArray apply(const QByteArray& data, const PDFObject& parameters, const PDFSecurityHandler* securityHandler) const
    {
        return apply(data, [](const PDFObject& object) -> const PDFObject& { return object; }, parameters, securityHandler);
    }
};

class PDFFORQTLIBSHARED_EXPORT PDFAsciiHexDecodeFilter : public PDFStreamFilter
{
public:
    explicit PDFAsciiHexDecodeFilter() = default;
    virtual ~PDFAsciiHexDecodeFilter() override = default;

    virtual QByteArray apply(const QByteArray& data,
                             const PDFObjectFetcher& objectFetcher,
                             const PDFObject& parameters,
                             const PDFSecurityHandler* securityHandler) const override;
};

class PDFFORQTLIBSHARED_EXPORT PDFAscii85DecodeFilter : public PDFStreamFilter
{
public:
    explicit PDFAscii85DecodeFilter() = default;
    virtual ~PDFAscii85DecodeFilter() override = default;

    virtual QByteArray apply(const QByteArray& data,
                             const PDFObjectFetcher& objectFetcher,
                             const PDFObject& parameters,
                             const PDFSecurityHandler* securityHandler) const override;
};

class PDFFORQTLIBSHARED_EXPORT PDFLzwDecodeFilter : public PDFStreamFilter
{
public:
    explicit PDFLzwDecodeFilter() = default;
    virtual ~PDFLzwDecodeFilter() override = default;

    virtual QByteArray apply(const QByteArray& data,
                             const PDFObjectFetcher& objectFetcher,
                             const PDFObject& parameters,
                             const PDFSecurityHandler* securityHandler) const override;
};

class PDFFORQTLIBSHARED_EXPORT PDFFlateDecodeFilter : public PDFStreamFilter
{
public:
    explicit PDFFlateDecodeFilter() = default;
    virtual ~PDFFlateDecodeFilter() override = default;

    virtual QByteArray apply(const QByteArray& data,
                             const PDFObjectFetcher& objectFetcher,
                             const PDFObject& parameters,
                             const PDFSecurityHandler* securityHandler) const override;

private:
    static QByteArray uncompress(const QByteArray& data);
};

class PDFFORQTLIBSHARED_EXPORT PDFRunLengthDecodeFilter : public PDFStreamFilter
{
public:
    explicit PDFRunLengthDecodeFilter() = default;
    virtual ~PDFRunLengthDecodeFilter() override = default;

    virtual QByteArray apply(const QByteArray& data,
                             const PDFObjectFetcher& objectFetcher,
                             const PDFObject& parameters,
                             const PDFSecurityHandler* securityHandler) const override;
};

class PDFFORQTLIBSHARED_EXPORT PDFCryptFilter : public PDFStreamFilter
{
public:
    explicit PDFCryptFilter() = default;
    virtual ~PDFCryptFilter() override = default;

    virtual QByteArray apply(const QByteArray& data,
                             const PDFObjectFetcher& objectFetcher,
                             const PDFObject& parameters,
                             const PDFSecurityHandler* securityHandler) const override;
};

}   // namespace pdf

#endif // PDFSTREAMFILTERS_H
